# USD Automated Testing

## Overview
USD Automated Testing is a command-line utility for validating USD files. This tool ensures your USD assets comply with basic standards by checking the validity of:
- Geometry (usdGeom)
- Shaders (usdShade)
- Layer structure

This program is built using Pixar's Universal Scene Description (USD) libraries, making it ideal for workflows in animation, VFX, and gaming pipelines.

---

## Prerequisites
Before building or using a prebuilt binary and then running this project, ensure you have the following:

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
   ngrok http --url=[your-generated-url].ngrok-free.app 8080
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
On your Windows machine, run the following commands to set up the Jenkins build agent:

```bash
java -jar agent.jar -url <your-ngrok-url> -secret @secret-file -name "windows_agent" -webSocket -workDir "C:\jenkins\agent"
```
Make sure to replace the secret key with the one generated by your Jenkins server.
---

## Usage
The usdTestRunner utility validates USD files by checking for essential components like geometry and shaders.

### Running the Test Runner
Use the following command to run the test runner:
```
./usdTestRunner path/to/usdfile.usda
```

---

### Example Output
```
Opened USD file Successfully.

[PASS] Validate Geometry: All geometry prims are valid.
[PASS] Validate Shader: All shaders in the scene are valid.
[PASS] Validate Layer Structure: Layer stack is valid.

Summary:
  Passed: 3
  Failed: 0

Congratulations, all tests were successful!
```
---

## Future Development Ideas
Ideas include:
- Allow users to run specific tests (e.g., --only-geometry, --skip-shaders) for greater flexibility.
- Save validation results to a text or JSON file for easier sharing and review.
- Expand to support additional evaluation types, such as asset hierarchy validation or custom plugin integrations.
- Add more test files to handle edge cases, ensuring robustness against unusual or invalid USD configurations.

---

## Contributing
Contributions are welcome! Feel free to submit issues or pull requests to enhance the functionality. When contributing, ensure you follow the project's coding style. Submit PRs with detailed descriptions of changes.

## Acknowledgments
- [Pixar Universal Scene Description](https://github.com/PixarAnimationStudios/USD)

## Learn More
If you're interested in learning more about the development of USD Automated Testing, including tips and challenges faced during implementation, check out [this blog post](https://luke-o.medium.com/c-automated-testing-framework-for-pixars-usd-50af70e58563).
