#include "CameraControllerLayer.h"
#include "core/IWindow.h"
#include "core/events/MouseEvent.h"
#include "core/events/Event.h"
#include "core/Log.h"
#include "components/CameraComponent.h"
#include "components/TransformComponent.h"
#include "CameraControllers.h"
#include "core/InputManager.h" // added
#include "core/KeyCodes.h"

#include <imgui.h>
#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RDE {
    using namespace Camera;

    CameraControllerLayer::CameraControllerLayer(entt::registry &registry, IWindow *window)
        : m_registry(registry), m_window(window), m_camera_entity(entt::null), m_proj_params_ptr(nullptr) {
        if (!m_window) {
            RDE_CORE_ERROR("CameraControllerLayer requires a valid window instance");
        }
        // Initialize view parameters with default values
        m_view_params.position = {0.0f, 0.0f, 5.0f};
        m_view_params.forward = {0.0f, 0.0f, -1.0f};
        m_view_params.up = {0.0f, 1.0f, 0.0f};
    }

    // --- Internal helpers implemented as member functions ---

    bool CameraControllerLayer::capture_events() const {
        if(!m_enable_input) return true; // treat as captured so we ignore
        ImGuiIO &io = ImGui::GetIO();
        if(!m_ignore_imgui_capture && (io.WantCaptureMouse || io.WantCaptureKeyboard)) return true;
        return false;
    }

    void CameraControllerLayer::sync_from_components() {
        // Acquire primary camera entity if changed
        entt::entity primary = CameraUtils::GetCameraEntityPrimary(m_registry);
        if (primary == entt::null) { m_camera_entity = entt::null; return; }
        if (primary != m_camera_entity) {
            m_camera_entity = primary;
            // Accept either unified CameraComponent or legacy CameraProjectionParameters
            bool hasUnified = m_registry.all_of<CameraComponent, TransformLocal>(m_camera_entity);
            bool hasLegacy = m_registry.all_of<CameraProjectionParameters, TransformLocal>(m_camera_entity);
            if (hasUnified || hasLegacy) {
                // Projection pointer setup
                if (hasUnified) {
                    auto &camComp = m_registry.get<CameraComponent>(m_camera_entity);
                    m_proj_params_ptr = &camComp.projection_params;
                } else {
                    m_proj_params_ptr = &m_registry.get<CameraProjectionParameters>(m_camera_entity); // reinterpret via typedef
                }
                auto &tl = m_registry.get<TransformLocal>(m_camera_entity);
                m_view_params.position = tl.translation;
                m_view_params.forward = glm::normalize(tl.orientation * glm::vec3(0,0,-1));
                m_view_params.up = glm::normalize(tl.orientation * glm::vec3(0,1,0));
                float radius = glm::max(glm::length(tl.translation), 1.0f);
                m_trackball = std::make_unique<TrackballController>(m_view_params, glm::vec3(0.0f), radius);
                m_trackball->view_all();
            }
        }
    }

    void CameraControllerLayer::sync_to_components() {
        if (m_camera_entity == entt::null) return;
        if (!m_registry.valid(m_camera_entity)) return;
        if (!m_registry.all_of<TransformLocal, CameraComponent>(m_camera_entity)) return;
        auto &tl = m_registry.get<TransformLocal>(m_camera_entity);
        // Recalculate orthonormal basis from forward/up
        glm::vec3 f = glm::normalize(m_view_params.forward);
        if (glm::length(f) < 1e-6f) f = {0,0,-1};
        glm::vec3 r = glm::normalize(glm::cross(f, m_view_params.up));
        if (glm::length(r) < 1e-6f) r = {1,0,0};
        glm::vec3 u = glm::normalize(glm::cross(r, f));
        glm::mat3 rot(r, u, -f); // Right-handed view basis
        tl.orientation = glm::quat_cast(rot);
        tl.translation = m_view_params.position;
        CameraUtils::SetCameraDirty(m_registry, m_camera_entity);
    }

    // --- ILayer overrides ---

    void CameraControllerLayer::on_attach() {
        sync_from_components();
    }

    void CameraControllerLayer::on_detach() {
        m_trackball.reset();
        m_camera_entity = entt::null;
        m_proj_params_ptr = nullptr;
    }

    void CameraControllerLayer::on_update(float delta_time) {
        sync_from_components();
        if(!m_enable_input) return;

        // Fly movement with WASD (and QE up/down) when in Fly mode
        if(m_mode == Mode::Fly && m_camera_entity != entt::null) {
            // Only move when RMB held, to avoid unintended drift while using UI
            if(m_right_down) {
                float speed = m_fly_speed;
                if(InputManager::is_key_held(KEY_LEFT_SHIFT) || InputManager::is_key_held(KEY_RIGHT_SHIFT)) speed *= 4.0f;
                glm::vec3 right = glm::normalize(glm::cross(m_view_params.forward, m_view_params.up));
                glm::vec3 move{0};
                if(InputManager::is_key_held(KEY_W)) move += m_view_params.forward;
                if(InputManager::is_key_held(KEY_S)) move -= m_view_params.forward;
                if(InputManager::is_key_held(KEY_D)) move += right;
                if(InputManager::is_key_held(KEY_A)) move -= right;
                if(InputManager::is_key_held(KEY_E)) move += m_view_params.up;   // up
                if(InputManager::is_key_held(KEY_Q)) move -= m_view_params.up;   // down
                if(glm::length(move) > 1e-6f) {
                    m_view_params.position += glm::normalize(move) * speed * delta_time;
                    sync_to_components();
                }
            }
        }

        if(m_dirty_gui) { // GUI changed values -> push to components
            sync_to_components();
            m_dirty_gui = false;
        }
    }

    void CameraControllerLayer::on_event(Event &e) {
        if (capture_events()) return; // UI has priority
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent &ev){ return on_mouse_move(ev); });
        dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent &ev){ return on_mouse_button_pressed(ev); });
        dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent &ev){ return on_mouse_button_released(ev); });
        dispatcher.dispatch<MouseScrolledEvent>([this](MouseScrolledEvent &ev){ return on_mouse_scrolled(ev); });
    }

    // --- Event handlers ---

    bool CameraControllerLayer::on_mouse_button_pressed(MouseButtonPressedEvent &e) {
        if (!m_enable_input) return false;
        if (m_camera_entity == entt::null) return false;
        // Use InputManager for current cursor position
        auto ci = InputManager::get_cursor_info();
        m_prev_mouse = ci.current_position;
        if (e.is_left_button()) {
            m_left_down = true;
            if(m_mode == Mode::Trackball && m_trackball) {
                int w,h; m_window->get_framebuffer_size(w,h);
                m_trackball->begin_rotate(m_prev_mouse, w, h);
            }
            return true;
        }
        if (e.is_middle_button()) { m_middle_down = true; return true; }
        if (e.is_right_button()) { m_right_down = true; return true; }
        return false;
    }

    bool CameraControllerLayer::on_mouse_button_released(MouseButtonReleasedEvent &e) {
        bool consumed = false;
        if (e.is_left_button()) { m_left_down = false; if (m_trackball) m_trackball->end_rotate(); consumed = true; }
        if (e.is_middle_button()) { m_middle_down = false; consumed = true; }
        if (e.is_right_button()) { m_right_down = false; consumed = true; }
        return consumed;
    }

    bool CameraControllerLayer::on_mouse_move(MouseMovedEvent &) {
        if (!m_enable_input) return false;
        if (m_camera_entity == entt::null) return false;
        // Always fetch latest position from InputManager instead of event payload
        glm::vec2 cur = InputManager::get_cursor_info().current_position;
        int w,h; m_window->get_framebuffer_size(w,h);
        if(m_mode == Mode::Trackball) {
            if (m_left_down && !m_middle_down && m_trackball) {
                m_trackball->update_rotate(cur, w, h);
                sync_to_components();
                m_prev_mouse = cur;
                return true;
            }
            if (m_middle_down && m_trackball) {
                glm::vec2 delta = cur - m_prev_mouse;
                m_trackball->pan(delta.x, delta.y);
                sync_to_components();
                m_prev_mouse = cur;
                return true;
            }
        } else { // Fly look with RMB
            if(m_right_down) {
                glm::vec2 delta = cur - m_prev_mouse;
                // Apply yaw (around up) and pitch (around right)
                float yaw_deg = delta.x * m_look_sensitivity;
                float pitch_deg = -delta.y * m_look_sensitivity;
                glm::vec3 right = glm::normalize(glm::cross(m_view_params.forward, m_view_params.up));
                // Build rotations in world space
                glm::mat4 yawM = glm::rotate(glm::mat4(1.f), glm::radians(yaw_deg), m_view_params.up);
                glm::mat4 pitchM = glm::rotate(glm::mat4(1.f), glm::radians(pitch_deg), right);
                glm::vec3 f = glm::normalize(glm::vec3(yawM * pitchM * glm::vec4(m_view_params.forward, 0.f)));
                // Avoid flipping: constrain pitch to not align with up
                if(glm::abs(glm::dot(f, m_view_params.up)) < 0.99f) {
                    m_view_params.forward = f;
                    // Recompute right/up to keep orthonormal
                    right = glm::normalize(glm::cross(m_view_params.forward, m_view_params.up));
                    m_view_params.up = glm::normalize(glm::cross(right, m_view_params.forward));
                    sync_to_components();
                }
                m_prev_mouse = cur;
                return true;
            }
        }
        m_prev_mouse = cur;
        return false;
    }

    bool CameraControllerLayer::on_mouse_scrolled(MouseScrolledEvent &e) {
        if (!m_enable_input) return false;
        if (m_camera_entity == entt::null) return false;
        if(m_mode == Mode::Trackball && m_trackball) {
            m_trackball->dolly(e.get_y_offset());
            sync_to_components();
            return true;
        }
        if(m_mode == Mode::Fly) {
            // Adjust fly speed using scroll
            m_fly_speed *= (e.get_y_offset() > 0 ? 1.1f : 0.9f);
            m_fly_speed = glm::clamp(m_fly_speed, 0.1f, 200.0f);
            return true;
        }
        return false;
    }

    void CameraControllerLayer::on_render_gui() {
        if(ImGui::Begin("Camera")) {
            ImGui::Checkbox("Enable Input", &m_enable_input);
            ImGui::SameLine();
            ImGui::Checkbox("Ignore ImGui Capture", &m_ignore_imgui_capture);
            int mode = (m_mode == Mode::Trackball ? 0 : 1);
            if(ImGui::RadioButton("Trackball", mode==0)) { m_mode = Mode::Trackball; }
            ImGui::SameLine();
            if(ImGui::RadioButton("Fly", mode==1)) { m_mode = Mode::Fly; }
            if(m_mode == Mode::Fly) {
                ImGui::DragFloat("Fly Speed", &m_fly_speed, 0.1f, 0.1f, 200.f, "%.2f");
                ImGui::DragFloat("Look Sens.", &m_look_sensitivity, 0.01f, 0.01f, 2.0f, "%.2f");
                ImGui::TextDisabled("RMB to look, WASD to move, Q/E down/up, Shift = turbo");
            }
            if(m_camera_entity == entt::null) {
                ImGui::TextDisabled("No primary camera");
            } else {
                ImGui::SeparatorText("View Parameters");
                m_dirty_gui |= ImGui::DragFloat3("Position", &m_view_params.position.x, 0.01f);
                m_dirty_gui |= ImGui::DragFloat3("Forward", &m_view_params.forward.x, 0.01f);
                m_dirty_gui |= ImGui::DragFloat3("Up", &m_view_params.up.x, 0.01f);
                if(ImGui::Button("Normalize Axes")) {
                    m_view_params.forward = glm::normalize(m_view_params.forward);
                    // Recompute up orthogonal
                    glm::vec3 right = glm::normalize(glm::cross(m_view_params.forward, m_view_params.up));
                    if(glm::length(right) < 1e-6f) right = glm::vec3(1,0,0);
                    m_view_params.up = glm::normalize(glm::cross(right, m_view_params.forward));
                    m_dirty_gui = true;
                }
                ImGui::SameLine();
                if(ImGui::Button("View All") && m_trackball) { m_trackball->view_all(); m_dirty_gui = true; }
                ImGui::SameLine();
                if(ImGui::Button("Reset") ) {
                    m_view_params.position = {0,0,5};
                    m_view_params.forward = {0,0,-1};
                    m_view_params.up = {0,1,0};
                    if(m_trackball) m_trackball->set_scene(glm::vec3(0), 5.f);
                    m_dirty_gui = true;
                }
                ImGui::SeparatorText("Projection");
                if(m_proj_params_ptr) {
                    auto &pp = *m_proj_params_ptr;
                    bool isPerspective = std::holds_alternative<CameraProjectionParameters::Perspective>(pp.parameters);
                    if(ImGui::Checkbox("Perspective", &isPerspective)) {
                        if(isPerspective) pp.parameters = CameraProjectionParameters::Perspective{}; else pp.parameters = CameraProjectionParameters::Orthographic{};
                        m_dirty_gui = true;
                    }
                    if(isPerspective) {
                        auto &persp = std::get<CameraProjectionParameters::Perspective>(pp.parameters);
                        m_dirty_gui |= ImGui::DragFloat("FOV (deg)", &persp.fov_degrees, 0.1f, 1.f, 170.f);
                        int w,h; m_window->get_framebuffer_size(w,h);
                        float aspect = (float)w / (float)h;
                        if(ImGui::DragFloat("Aspect", &persp.aspect_ratio, 0.001f, 0.1f, 8.f)) m_dirty_gui = true;
                        ImGui::SameLine(); if(ImGui::Button("Auto Aspect")) { persp.aspect_ratio = aspect; m_dirty_gui = true; }
                    } else {
                        auto &ortho = std::get<CameraProjectionParameters::Orthographic>(pp.parameters);
                        m_dirty_gui |= ImGui::DragFloat("Left", &ortho.left, 0.01f);
                        m_dirty_gui |= ImGui::DragFloat("Right", &ortho.right, 0.01f);
                        m_dirty_gui |= ImGui::DragFloat("Bottom", &ortho.bottom, 0.01f);
                        m_dirty_gui |= ImGui::DragFloat("Top", &ortho.top, 0.01f);
                    }
                    m_dirty_gui |= ImGui::DragFloat("Near", &pp.near_plane, 0.001f, 0.001f, pp.far_plane - 0.01f);
                    m_dirty_gui |= ImGui::DragFloat("Far", &pp.far_plane, 0.1f, pp.near_plane + 0.01f, 10000.f);
                }
            }
        }
        ImGui::End();
    }
}

