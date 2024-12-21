/**
 * @file main.cpp
 * @brief Validates USD files by checking geometry, shaders, and layer structure for correctness.
 *
 * This program uses the USD API to validate USD files against basic asset standards:
 * - Checks geometry presence and validity.
 * - Verifies shader definitions.
 * - Validates layer structure.
 * Outputs test results and a summary of passed and failed checks.
 *
 * Dependencies:
 * - Pixar USD Library (pxr namespace)
 * - Standard C++ Libraries (iostream, vector, string, functional)
 */

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdShade/shader.h>
#include <iostream>
#include <vector>
#include <string>
#include <functional>

/**
 * @struct TestResult
 * @brief Structure to represent the result of a single test.
 *
 * @var testName
 * The name of the test being executed.
 *
 * @var passed
 * Indicates whether the test passed (true) or failed (false).
 *
 * @var message
 * Additional information about the test result.
 */
struct TestResult {
    std::string testName;
    bool passed;
    std::string message;
};

/**
 * @typedef ValidationFunction
 * @brief A type alias for a validation function that takes a UsdStageRefPtr and returns a TestResult.
 */
using ValidationFunction = std::function<TestResult(const pxr::UsdStageRefPtr&)>;

/**
 * @class TestRunner
 * @brief Manages and executes validation tests on USD files.
 *
 * Provides methods to:
 * - Add validation tests.
 * - Execute tests on the specified USD file.
 * - Summarize results of all tests.
 */
class TestRunner {
public:
    /**
     * @brief Constructs a TestRunner with the specified USD file path.
     * @param filePath The path to the USD file to be tested.
     */
    explicit TestRunner(const std::string& filePath) : usdFilePath(filePath) {}

    /**
     * @brief Adds a validation test to the test runner.
     * @param test The validation function to be added.
     */
    void addTest(const ValidationFunction& test) {
        tests.push_back(test);
    }

    /**
     * @brief Executes all added tests on the USD file.
     * 
     * Always inform user when attempting to run tests regardless if successful.
     */
    void runTests() {
        pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(usdFilePath);
        if (!stage) {
            std::cerr << "Failed to open USD file. Ensure the file path is correct and the file is accessible.\n\n";
            return;
        } else {
            std::cout << "Opened USD file Successfully.\n\n";
        }

        for (const auto& test : tests) {
            TestResult result = test(stage);
            report(result);
        }

        summarize();
    }

private:
    std::string usdFilePath; ///< The path to the USD file.
    std::vector<ValidationFunction> tests; ///< List of validation functions to execute.
    std::vector<TestResult> results; ///< Results of the executed tests.

    /**
     * @brief Logs the result of a test.
     * @param result The result of the test to be logged.
     */
    void report(const TestResult& result) {
        results.push_back(result);
        std::cout << "[" << (result.passed ? "PASS" : "FAIL") << "] "
                  << result.testName << ": " << result.message << "\n";
    }

    /**
     * @brief Summarizes the test results, displaying the count of passed and failed tests.
     */
    void summarize() {
        int passed = 0, failed = 0;
        for (const auto& result : results) {
            if (result.passed) {
                ++passed;
            } else {
                ++failed;
            }
        }

        std::cout << "\nSummary:\n";
        std::cout << "  Passed: " << passed << "\n";
        std::cout << "  Failed: " << failed << "\n";

        if (failed > 0 && passed > 0) {
            std::cout << "\nSome tests failed. Please review the USD file and address the failing tests.\n\n";
        } else if (failed > 0) {
            std::cout << "\nAll tests failed. The USD file may have serious issues. Please review it thoroughly.\n\n";
        } else {
            std::cout << "\nCongratulations, all tests were successful!\n\n";
        }
    }
};

/**
 * @brief Validates the presence and correctness of geometry in the USD file.
 * @param stage The USD stage to examine.
 * @return TestResult with the validation outcome and details.
 */
TestResult validateGeometry(const pxr::UsdStageRefPtr& stage) {
    auto rootPrim = stage->GetPrimAtPath(pxr::SdfPath("/"));
    if (!rootPrim) {
        return {"Validate Geometry", false, "No root prim found in the scene."};
    }

    bool hasValidGeometry = false;

    for (auto prim : stage->Traverse()) {
        if (!prim.IsValid()) {
            return {"Validate Geometry", false, "Encountered an invalid prim in the scene: " + prim.GetPath().GetString()};
        }

        // Check if the prim has meaningful data (attributes or children)
        if (!prim.GetAttributes().empty() || !prim.GetChildren().empty()) {
            hasValidGeometry = true;
        }
    }

    if (!hasValidGeometry) {
        return {"Validate Geometry", false, "No valid geometry prims found in the scene."};
    }

    return {"Validate Geometry", true, "All geometry prims are valid."};
}

/**
 * @brief Validates the presence and correctness of shaders in the USD file.
 * @param stage The USD stage to examine.
 * @return TestResult with the validation outcome and details.
 */
TestResult validateShaders(const pxr::UsdStageRefPtr& stage) {
    auto rootPrim = stage->GetPrimAtPath(pxr::SdfPath("/"));
    if (!rootPrim || rootPrim.GetChildren().empty()) {
        return {"Validate Shaders", false, "No shaders found in the scene."};
    }

    bool foundValidShader = false;

    for (pxr::UsdPrim prim : stage->Traverse()) {
        if (prim.GetTypeName() == "Shader") {
            foundValidShader = true;

            if (!prim.IsValid()) {
                return {"Validate Shaders", false, "Invalid shader found at: " + prim.GetPath().GetString()};
            }
        }
    }

    if (!foundValidShader) {
        return {"Validate Shaders", false, "No valid shaders found in the scene."};
    }

    return {"Validate Shaders", true, "All shaders in the scene are valid."};
}

/**
 * @brief Checks the structure of layers in the USD file.
 * @param stage The USD stage to validate.
 * @return TestResult with the validation result and details.
 */
TestResult validateLayerStructure(const pxr::UsdStageRefPtr& stage) {
    auto layerStack = stage->GetLayerStack();

    if (layerStack.empty()) {
        return {"Validate Layer Structure", false, "Layer stack is empty."};
    }

    for (size_t i = 0; i < layerStack.size(); ++i) {
        const auto& layer = layerStack[i];
        if (!layer) {
            return {"Validate Layer Structure", false, "Broken reference at layer index " + std::to_string(i) + "."};
        }

        // Check for unresolved sublayers
        for (const auto& subLayerPath : layer->GetSubLayerPaths()) {
            auto subLayer = pxr::SdfLayer::FindOrOpen(subLayerPath);
            if (!subLayer) {
                return {"Validate Layer Structure", false, "Unresolved sublayer: " + std::string(subLayerPath)};
            }
        }

        // Check if the root prim exists in this layer
        auto rootPrimSpec = layer->GetPrimAtPath(pxr::SdfPath("/"));
        if (!rootPrimSpec) {
            return {"Validate Layer Structure", false, "Layer at index " + std::to_string(i) + " has no root prim."};
        }
    }

    return {"Validate Layer Structure", true, "Layer stack is valid."};
}

/**
 * @brief Displays the program introduction with ASCII art and a description of the tool.
 */
void displayIntro() {
    std::cout << R"(
   ___      ___       ________      _________
  |   |    |   |    /   ___   \    |         \
  |   |    |   |   |   /   \___|   |    ___   \
  |   |    |   |   |   \______     |   |   |   |
  |   |    |   |    \______   \    |   |   |   |
  |   |____|   |    ___    \   \   |   |___|   |
  |            |   |   \___/   |   |          /
   \__________/     \_________/    |_________/
    )" << '\n';

    std::cout << "\nWelcome to the USD Test Runner!\n";
    std::cout << "This program validates USD files for geometry, shaders, and layer structure.\n";
    std::cout << "Provide a USD file as input to test its compliance with basic asset standards.\n";
    std::cout << "\nFor further details, view the README file located in the root directory.\n";
    std::cout << "------------------------------------------------------------\n\n";
}

/**
 * @brief Entry point for the program.
 *
 * @param argc The number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Exit status of the program.
 */
int main(int argc, char* argv[]) {

    if (argc < 2) {
        displayIntro();
        std::cerr << "Usage: " << argv[0] << " <path-to-usd-file>\n";
        return 1;
    }

    const std::string usdFilePath = argv[1];
    TestRunner runner(usdFilePath);

    runner.addTest(validateGeometry);
    runner.addTest(validateShaders);
    runner.addTest(validateLayerStructure);

    runner.runTests();

    return 0;
}
