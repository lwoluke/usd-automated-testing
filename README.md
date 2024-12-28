# USD Automated Testing

## Overview
USD Automated Testing is a command-line utility for validating USD files. This tool ensures your USD assets comply with basic standards by checking the validity of:
- Geometry (usdGeom)
- Shaders (usdShade)
- Layer structure
- Variants

This program is built using Pixar's Universal Scene Description (USD) libraries, making it ideal for workflows in animation, VFX, and gaming pipelines.

---

## Prerequisites
Before building or using a prebuilt binary, and then running this project, ensure you have the following:

### Required Tools
- A C++ compiler (e.g., GCC, Clang, MSVC)
- [CMake](https://cmake.org/download/) (version 3.16 or later)

---

## Setup Instructions

### Step 1: Clone the Repository
Clone this repository to your local machine:
```
git clone https://github.com/lwoluke/automated-usd-testing.git
cd automated-usd-testing
```

### Step 2: Create USD build or Download Prebuilt Binary
Choice #1: Follow the [official USD build instructions](https://github.com/PixarAnimationStudios/OpenUSD/tree/release?tab=readme-ov-file#getting-and-building-the-code) to set up the USD libraries.

Choice #2: Visit the [Nvidia Pre-Built OpenUSD Download](https://developer.nvidia.com/usd) section to install the USD libraries.

Make sure the USD libraries are either built or downloaded, and then placed into the `usd` directory within the project.

### Step 3: Build the Project on Your Machine
Navigate to the build directory and compile the project:
```
cd build
cmake ..
make
```
This will produce the `usdTestRunner` so you can test USD files on the command line.

---
## Continuous Integration Setup

To facilitate automated testing and building, you'll need to set up a Jenkins server along with build agents for both Windows and Linux environments. Below are the instructions to get everything up and running.

### Jenkins Server Setup

While I have configured Jenkins locally for my setup, you'll need to set up your own Jenkins server. Using the commands provided below can help streamline this process.

### Initial Installation

   Follow the [official Jenkins installation guide](https://www.jenkins.io/doc/book/installing/) to install Jenkins on your preferred platform.

### External Access Configuration

   To make your local Jenkins server accessible, use ngrok:

   ```bash
   ngrok http --url=<your-ngrok-url> 8080
   ```
   To access your free static url, go to the [Ngrok Setup Page](https://dashboard.ngrok.com/get-started/setup/windows)

### Build Agent Configuration
You'll need to run build agents on both Linux and Windows to enable Jenkins to execute builds on these platforms.

### Linux Build Agent Setup
Execute the following command on your Linux machine to start the Jenkins build agent:

```bash
java -jar agent.jar -url <your-ngrok-url> -secret @secret-file -name "linux_agent" -webSocket -workDir "/home/jenkins/agent"
```
Ensure that agent.jar is downloaded from your Jenkins server and that the secret-file contains the appropriate secret key provided by Jenkins.

### Windows Build Agent Setup
On your Windows machine, run the following commands to set up the Jenkins build agent. Make sure to replace the secret key with the one generated by your Jenkins server:

```bash
java -jar agent.jar -url <your-ngrok-url> -secret @secret-file -name "windows_agent" -webSocket -workDir "C:\jenkins\agent"
```

---

## Usage
The usdTestRunner utility validates USD files by checking for essential components like geometry and shaders.

### Running the Test Runner
Examples of commands to run the test runner:
```
# Run all tests (default behavior)
./usdTestRunner path/to/file.usda

# Run only geometry validation
./usdTestRunner path/to/file.usda -only-geometry

# Skip shader validation
./usdTestRunner path/to/file.usda -skip-shaders

# Skip multiple tests
./usdTestRunner path/to/file.usda -skip-geometry -skip-shaders

# To see a full list of commands, run:
./usdTestRunner -help

# Run tests and save results to a file
./usdTestRunner path/to/file.usda -output results.txt

# Run specific tests and save results
./usdTestRunner path/to/file.usda -only-geometry -output geometry_results.txt

# Skip tests and save results
./usdTestRunner path/to/file.usda -skip-shaders -output validation_results.txt
```

---

### Example Output
```
Opened USD file Successfully.

[PASS] Validate Geometry: No geometry found in the scene, but that's not required.
[PASS] Validate Shaders: No shaders found in the scene, but that's acceptable.
[PASS] Validate Layer Structure: Layer stack and all references are valid.
[PASS] Validate Variants: No variants found in the scene. That's acceptable.

Summary:
  Passed: 4
  Failed: 0

Congratulations, all tests were successful!

```
---

## Contributing
Contributions are welcome! Feel free to submit issues or pull requests to enhance the functionality. When contributing, ensure you follow the project's coding style. Submit PRs with detailed descriptions of changes.

---

## Acknowledgments
- [Pixar Universal Scene Description](https://github.com/PixarAnimationStudios/USD)

---

## Learn More
If you're interested in learning more about the development of USD Automated Testing, including tips and challenges faced during implementation, check out this [project part 1 blog post](https://luke-o.medium.com/c-automated-testing-framework-for-pixars-usd-50af70e58563), this [project part 2 blog post](https://luke-o.medium.com/extending-usd-automated-testing-with-jenkins-integration-002dcdec3786), and this [project part 3 blog post](https://luke-o.medium.com/usd-automated-testing-tool-functionality-expansion-33d23e06ea34).
