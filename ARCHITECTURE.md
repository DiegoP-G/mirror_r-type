# Project Structure
- `client/`: client-side source code
- `server/`: server-side source code
- `ecs/`: entity-component system, used by client and server
- `transferData/`: methods to serialize, de-serialize data, used by client and server
- `assets/`: assets of the project
- `doc/`: technical deocumentation of the project
- `submodules`: git submodule of the project (SFML, cmake, ...)



## Modules
...

## Transfer protocal
The project use TCP and UDP transfer, see technical documentation
For more details of the protocol, see [`doc/protocol.txt`](doc/protocol.txt).

## Design Paterns
The `mediator` design patern is used in the server side.

## External Dependencies
- SFML
- CMake