The Layer's Job: 

    To manipulate the state of the Scene. 
    It creates entities, moves them, changes their components, and responds to game events. 
    It describes what the world should look like.

The RenderingSystem's Job: 

    To look at the Scene data (all the TransformComponents, RenderableComponents, etc.) and 
    translate that declarative state into a series of imperative draw commands for the IRenderer. 
    It decides how to draw the world.