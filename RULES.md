# Project RULES:


## Repository rules:
* "master" branch is a stable branch.
* "new feature" equals "new branch".
* file naming convention: "file.c" for C file. "file.cc" for C++ file.
* project files naming convention: "Project.pro" for main project definition. "project.pro" for source side project details ("project.pro" requires "src/project/project.pro" due to qmake convention).
* indentation => 4 spaces.


## Project versioning rules:
* project version number increases when reach some milestone (usually remains "0" forever, cause project never ends by definition).
* major version number increases when some new feature is implemented.
* minor version number increases after small feature fix or patch.
* odd minor indicates experimental branch (for example: `0.5.1`).
* even major version number means stable - production ready version (for example: `0.2.1`)


## Commit rules:
* Each commit should contain one change, or one set of changes about exactly one issue/ feature.
