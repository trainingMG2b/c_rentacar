<div id="top"></div>

<!-- ABOUT THE PROJECT -->
## About The Project

This project is a placeholder for training C programming on Unix with GIT as an SCM.
The source files are kept **very** simple in order to remain accessible to **beginners**.

### Built With

The project uses C standard libraries on an Unix environnment.
A **Makefile** is used to build the project, and the Visual Studio Code .vscode folder contains basic launch, c properties and tasks json files in order to develop the project with a properly configured VSC with Microsoft C/C++ extension, Makefile extension and Hex file explorer extension.


<!-- GETTING STARTED -->
## Getting Started

### Prerequisites

To compile the project, you need an Unix environnement configured with the C compilatioon toolchain (usualy GCC). In the case those tools are not installed, please check your Unix distribution documentation for more information on how to install the C toolchain.

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/trainingMG2b/c_rentacar
   ```
2. In the project directory
   ```sh
   make all
   ```


<!-- USAGE EXAMPLES -->
## Usage

Usage is 
```sh
./server
```

In this version (v0.2.0), the program acts like a **very basic** server that listens on port 13722 on localhost for incoming **telnet** or (exclusive) **postman/curl** connections to process HTTP requests.  
In version v0.1.0, server mode introduced, and only telnet acces is functional.  
In version v0.0.0, commands where passed as arguments on the command line.
---
In telnet mode (with the -t|--telnet argument), the program processes one HTTP request at a time. Use a telnet client to send the request, line by line until all needed data is gathered by the server in order to process the request.  
The HTTP request should be passed first followed by the header lines as per RFC.  
For requests that need a json body (POST, PUT, PATCH), an empty line must be send first, followed by the json body.  
At last, an empty line triggers the request processing.
---
In standard mode (without the -t|--telnet argument), the program waits for whole request to arrive at once, and then processes the request. Use **postman** or **curl** clients to send requests.
---
As of version v0.2.0, only Car operations are handled.  
Json input files examples can be found in the json subdirectory.  
Look at the the launch.json file in the .vscode directory for some experimentation on creating/updating/deleting cars.


<!-- CONTRIBUTING -->
## Contributing

This is a project for training purposes only, please do not fork/contribute ;-), let this done by the students in their own GitHub project.


<!-- LICENSE -->
## License

Distributed under the Apache 2.0 License.
For more information about the Apache 2.0 License [follow this link](https://www.apache.org/licenses/LICENSE-2.0).



<!-- CONTACT -->
## Contact

Training MG2b - [@trainingMG2b](https://twitter.com/trainingMG2b)

Project Link: [https://github.com/trainingMG2b/c_rentacar](https://github.com/trainingMG2b/c_rentacar)

<p align="right">(<a href="#top">back to top</a>)</p>
