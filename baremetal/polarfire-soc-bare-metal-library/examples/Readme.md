# Bare Metal Library Example Projects
This document describes how to use the SoftConsole example projects included in
the PolarFire SoC Bare Metal Library.

## Important Note About SoftConsole Workspace
SoftConsole includes an empty workspace. We recommend that you make a copy of
this empty workspace to a location of your choice. This will allow you to take
advantage of the integrated "develop and debug" perpective, Renode platform
launchers and other preconfigured settings.

Using the empty workspace as a starting point for your own workspace allows the
example projects to work out of the box. In particular the Renode debug launchers
included in the example projects will refer to the correct Renode external tool
launchers.

The empty workspace is located under the following path of your SoftConsole
installation:

    ./extras/workspace.empty

## Importing Example Projects Into Your SoftConsole Workspace
SoftConsole example projects are include in the following folder of the of the
Bare Metal Library: mpfs-bare-metal-library/examples

- Select the File->Import menu to import an example project into your SoftConsole
Workspace:

   ![File_Import_menu](images/file_import_menu.png)


- Select "Existing Projects into Workspace" and click "Next":

   ![Import_Select_Dialog](images/import_select_dialog.png)

- Select the folder inside the Bare Metal Library containing the example project
of your choice then click "Select Folder".

   Ensure that the project of your choice is listed and the box beside it ticked
in the "Projects" list. If the project does not show up in the list, it probably
means that you selected the incorrect folder level in the previous step.
Ensure the "Copy projects into workspace" option is ticked then click "Finish".

   ![Import_Projects_Dialog](images/import_projects_dialog.png)

- The imported project now appears in the "Project Explorer":

   ![Imported_Project](images/imported_project.png)

## Executing/Debugging Examples In Renode
Most example projects include a Renode debug launcher allowing execution of the
example projects in the Renode virtual platform.

- Click on the down-arrow on the right of the "Debug" button then click on "Debug Configurations":

   ![Debug_Config](images/debug_config.png)

- Select a Renode launcher in the "Launch Group" list then click "debug":

   ![Debug_Configurations](images/debug_configurations.png)

- This will launch Renode and some Renode analyzers allowing you to interact with the executing example code:

   ![Renode_Example](images/renode_example.png)

- The debug perspective reflects the debug operations:

   ![Debug_Perspective](images/debug_perspective.png)

- The debug execution is controlled using the debug buttons:

   ![Debug_Buttons](images/debug_buttons.png)


