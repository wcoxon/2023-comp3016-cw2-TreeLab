# 2023 comp3016 cw2 TreeLab
 an interactive app and developer tool for procedurally generating meshes of trees which can be exported to OBJ model files. the user can edit the tree using UI widgets to control a selection of parameters in generating the tree.


▪ How does the user interact with your executable? How do you open and control the software you wrote (exe file)? 

The software is an executable file.
The user controls the software through a UI context menu, with widgets such as sliders for the different parameters. 
The view is also controlled by clicking and dragging the right and middle mouse buttons.

▪ How does the program code work? How do the classes and functions fit together and who does what? 

The Mesh class contains vertex attribute arrays and indices and provides functionality to bind these to OpenGL buffers and send them to the shaders, as well as to draw and update uniform values.
The mesh class also contains functionality for loading vertex and index data from OBJ files, as well as writing it’s own geometry data to a given file path. 

The Tree class stores properties used to generate a tree, and a list of nodes which contain radius and the number of branches from the node. It also stores a list of offsets for the first nodes of each generation, which is used when traversing the tree.

The tree class contains the outer mesh and leaves as members, and has methods for generating them on demand.

When generating the tree, first a single node will be created, and its index marked as the first generation of nodes in the tree, and then the generateNextLayer method is used to iterate through each generation’s nodes and branch new nodes from them into the next generation/layer. The radius of a node is propagated into its child nodes with a decrease applied to it, when a node has radius 0, it will no longer create new branches, ending the generation procedure.

The tick function contains all of the user interface logic and binds widgets to different variables, such as environmental effects like the direction of the light source and background colour, as well as hooking to properties of the tree and its meshes.

▪ What makes your program special and how does it compare to similar things? (Where did you get the idea from? What did you start with? How did you make yours unique? Did you start with a given project?) 

My program can be used for designing assets for environments such as in-game development or 3D arts. The user is given a high degree of control over the asset's structure and level of detail, allowing for a broad range of art styles, memory optimisations, and even species of trees to be produced using this application. 

I started with a project template provided by the module in the GitHub classroom repository for this assignment. 

libraries used:
 glfw.3.3.4
 imgui (master)
 glew.v140.1.12.0
 freeglut.3.0.0.v140.1.0.2
 glm.0.9.9.800
 irrKlang-64bit-1.6.0

▪ A Link to the video 
https://youtu.be/txb4-Rs3abc
