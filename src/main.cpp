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
 * Command line options:
 * -only-geometry    : Run only geometry validation
 * -only-shaders     : Run only shader validation
 * -only-layers      : Run only layer structure validation
 * -only-variants    : Run only variant validation
 * -skip-geometry    : Skip geometry validation
 * -skip-shaders     : Skip shader validation
 * -skip-layers      : Skip layer structure validation
 * -skip-variants    : Skip variant validation
 * -output <path>    : Export results to the specified file path
 * -help             : Display this help message
 * 
 * Dependencies:
 * - Pixar USD Library (pxr namespace)
 * - Standard C++ Libraries (iostream, vector, string, functional)
 */

#include "usdIncludes.h"

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
 * @struct TestConfig
 * @brief Configuration for which tests should be run
 */
struct TestConfig {
    bool runGeometry = true;
    bool runShaders = true;
    bool runLayers = true;
    bool runVariants = true;
    std::string outputPath;

    // Returns true if at least one test is enabled
    bool hasEnabledTests() const {
        return runGeometry || runShaders || runLayers || runVariants;
    }
};

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
     * @brief Adds a validation test to the test runner with an associated identifier.
     * @param id The identifier for the test
     * @param test The validation function to be added
     */
    void addTest(const std::string& id, const ValidationFunction& test) {
        tests.push_back({id, test});
    }

    /**
     * @brief Executes tests based on the provided configuration
     * @param config TestConfig specifying which tests to run
     */
    void runTests(const TestConfig& config) {
        // Clear previous results
        results.clear();
        output.str("");  // Clear the stringstream

        pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(usdFilePath);
        if (!stage) {
            std::string error = "Failed to open USD file. Ensure the file path is correct and the file is accessible.\n\n";
            std::cerr << error;
            output << error;
            return;
        } else {
            std::string success = "Opened USD file Successfully.\n\n";
            std::cout << success;
            output << success;
        }

        for (const auto& [id, test] : tests) {
            if ((id == "geometry" && config.runGeometry) ||
                (id == "shaders" && config.runShaders) ||
                (id == "layers" && config.runLayers) ||
                (id == "variants" && config.runVariants)) {
                TestResult result = test(stage);
                report(result);
            }
        }

        summarize();

        // Export results if output path is specified
        if (!config.outputPath.empty()) {
            exportResults(config.outputPath);
        }
    }

private:
    std::string usdFilePath; // The path to the USD file.
    std::vector<std::pair<std::string, ValidationFunction>> tests; // List of validation functions to execute.
    std::vector<TestResult> results; // Results of the executed tests.
    std::stringstream output;  // New member to collect output.

    /**
     * @brief Logs the result of a test.
     * @param result The result of the test to be logged.
     */
    void report(const TestResult& result) {
        results.push_back(result);
        std::string resultStr = "[" + std::string(result.passed ? "PASS" : "FAIL") + "] " 
                               + result.testName + ": " + result.message + "\n";
        std::cout << resultStr;
        output << resultStr;
    }

    /**
     * @brief Summarizes the test results, displaying the count of passed and failed tests.
     */
    void summarize() {
        int passed = 0, failed = 0;
        for (const auto& result : results) {
            if (result.passed) ++passed;
            else ++failed;
        }

        std::string summary = "\nSummary:\n"
                             "  Passed: " + std::to_string(passed) + "\n"
                             "  Failed: " + std::to_string(failed) + "\n\n";

        std::string conclusion;
        if (failed > 0 && passed > 0) {
            conclusion = "Some tests failed. Please review the USD file and address the failing tests.\n\n";
        } else if (failed > 0) {
            conclusion = "All tests failed. The USD file may have serious issues. Please review it thoroughly.\n\n";
        } else {
            conclusion = "Congratulations, all tests were successful!\n\n";
        }

        std::cout << summary << conclusion;
        output << summary << conclusion;
    }

    /**
     * @brief Outputs the test results to the location of the file path provided.
     */
    void exportResults(const std::string& filePath) {
        std::ofstream outFile(filePath);
        if (!outFile) {
            std::cerr << "Error: Could not open output file: " << filePath << "\n";
            return;
        }

        outFile << output.str();
        std::cout << "Results exported to: " << filePath << "\n";
    }
};

/**
 * @brief Validates the presence and correctness of geometry in a USD file.
 *
 * Ensures that all geometry prims (`UsdGeomXform` and `UsdGeomMesh`) are valid by:
 * - Verifying transform operations for `UsdGeomXform` prims.
 * - Checking `extent` attributes and point data for `UsdGeomMesh` prims.
 * - Detecting missing attributes and degenerate geometry.
 *
 * Reports invalid or incomplete geometry attributes. Passes if no geometry is found unless mandatory.
 *
 * @param stage The USD stage to validate.
 * @return TestResult Containing:
 *         - Test name ("Validate Geometry").
 *         - Success/failure status.
 *         - Detailed validation results or errors.
 */
TestResult validateGeometry(const pxr::UsdStageRefPtr& stage) {
    if (!stage) {
        return {"Validate Geometry", false, "Invalid stage reference."};
    }

    auto rootPrim = stage->GetPrimAtPath(pxr::SdfPath("/"));
    if (!rootPrim) {
        return {"Validate Geometry", false, "No root prim found in the scene."};
    }

    bool foundGeometryPrim = false;
    std::vector<std::string> errors;

    for (auto prim : stage->Traverse()) {
        if (!prim.IsValid()) {
            errors.push_back("Encountered an invalid prim in the scene: " + prim.GetPath().GetString());
            continue;
        }

        if (prim.IsA<pxr::UsdGeomXform>() || prim.IsA<pxr::UsdGeomMesh>()) {
            foundGeometryPrim = true;

            if (prim.IsA<pxr::UsdGeomXform>()) {
                auto xformable = pxr::UsdGeomXform(prim);
                bool resetXformStack;
                std::vector<pxr::UsdGeomXformOp> xformOps = xformable.GetOrderedXformOps(&resetXformStack);

                for (const auto& op : xformOps) {
                    if (!op.GetAttr()) {
                        errors.push_back("Invalid transform operation found at: " +
                                         prim.GetPath().GetString());
                    }
                }
            }

            if (auto mesh = pxr::UsdGeomMesh(prim)) {
                if (auto extentAttr = mesh.GetExtentAttr()) {
                    pxr::VtVec3fArray extentArray;
                    if (!extentAttr.Get(&extentArray)) {
                        errors.push_back("Invalid extent bounds at: " + prim.GetPath().GetString());
                    } else if (extentArray.size() == 2) {
                        const auto& min = extentArray[0];
                        const auto& max = extentArray[1];
                        if (min == max) {
                            errors.push_back("Degenerate geometry found at: " + prim.GetPath().GetString());
                        }
                    }
                } else {
                    errors.push_back("Extent missing for Mesh at path: " + prim.GetPath().GetString());
                }

                if (auto pointsAttr = mesh.GetPointsAttr()) {
                    pxr::VtVec3fArray pointArray;
                    if (!pointsAttr.Get(&pointArray)) {
                        errors.push_back("Invalid point data at: " + prim.GetPath().GetString());
                    }
                }
            }

        }
    }

    if (!foundGeometryPrim) {
        return {
            "Validate Geometry",
            true,
            "No geometry found in the scene, but that's not required."
        };
    }

    if (!errors.empty()) {
        std::string errorMsg = "Geometry validation failed with the following issues:\n";
        for (const auto& error : errors) {
            errorMsg += "- " + error + "\n";
        }
        return {"Validate Geometry", false, errorMsg};
    }

    return {"Validate Geometry", true, "All geometry prims are valid with proper transforms and bounds."};
}

/**
 * @brief Validates the presence and correctness of shaders in a USD file.
 *
 * Checks for valid shader prims (`UsdShadeShader`), ensuring:
 * - Proper shader IDs and input parameters.
 * - Valid connections to other shaders or materials.
 * - Presence of valid shader source asset paths.
 *
 * If no shaders are found, validation passes unless they are required.
 *
 * @param stage The USD stage to validate.
 * @return TestResult Containing:
 *         - Test name ("Validate Shaders").
 *         - Success/failure status.
 *         - Detailed validation results or errors.
 */
TestResult validateShaders(const pxr::UsdStageRefPtr& stage) {
    if (!stage) {
        return {"Validate Shaders", false, "Invalid stage reference."};
    }

    bool foundAnyShader = false;
    std::vector<std::string> errors;

    for (auto prim : stage->Traverse()) {
        if (!prim.IsValid()) {
            errors.push_back("Invalid prim encountered during shader validation: " + prim.GetPath().GetString());
            continue;
        }

        if (pxr::UsdShadeShader shader = pxr::UsdShadeShader(prim)) {
            foundAnyShader = true;

            pxr::TfToken shaderId;
            shader.GetShaderId(&shaderId);
            if (shaderId.IsEmpty()) {
                errors.push_back("Missing or invalid shader ID at: " + prim.GetPath().GetString());
            }

            auto inputs = shader.GetInputs();
            if (inputs.empty()) {
                errors.push_back("Shader has no input parameters at: " + prim.GetPath().GetString());
            } else {
                for (const auto& input : inputs) {
                    pxr::UsdShadeConnectableAPI source;
                    pxr::TfToken sourceName;
                    pxr::UsdShadeAttributeType sourceType;
                    if (input.GetConnectedSource(&source, &sourceName, &sourceType)) {
                        if (!source.GetPrim().IsValid()) {
                            errors.push_back("Invalid shader connection at: " +
                                             input.GetBaseName().GetString() + " on prim " +
                                             prim.GetPath().GetString());
                        }
                    }
                }
            }

            pxr::SdfAssetPath sourceAsset;
            if (shader.GetSourceAsset(&sourceAsset)) {
                if (sourceAsset.GetAssetPath().empty()) {
                    errors.push_back("Missing shader source asset path at: " + prim.GetPath().GetString());
                }
            }

            if (auto material = pxr::UsdShadeMaterial(prim.GetParent())) {
                auto surface = material.GetSurfaceOutput();
                if (surface) {
                    pxr::UsdShadeConnectableAPI source;
                    pxr::TfToken sourceName;
                    pxr::UsdShadeAttributeType sourceType;
                    if (surface.GetConnectedSource(&source, &sourceName, &sourceType)) {
                        if (!source.GetPrim().IsValid()) {
                            errors.push_back("Invalid material binding at: " + 
                                             prim.GetParent().GetPath().GetString());
                        }
                    }
                }
            }
        }
    }

    if (!foundAnyShader) {
        return {
            "Validate Shaders",
            true,
            "No shaders found in the scene, but that's acceptable."
        };
    }

    if (!errors.empty()) {
        std::string errorMsg = "Shader validation failed with the following issues:\n";
        for (const auto& e : errors) {
            errorMsg += "- " + e + "\n";
        }
        return {"Validate Shaders", false, errorMsg};
    }

    return {"Validate Shaders", true, "All shaders and their connections are valid."};
}

/**
 * @brief Validates the structure and integrity of layers in a USD file.
 *
 * Ensures the USD stage has a valid layer stack, checking:
 * - Presence of a root layer with a default prim.
 * - No duplicate layer identifiers.
 * - Resolving all sublayer paths, references, and payloads.
 * - Valid root prims in each layer where applicable.
 *
 * Reports unresolved sublayers, broken references, or missing root prims.
 *
 * @param stage The USD stage to validate.
 * @return TestResult Containing:
 *         - Test name ("Validate Layer Structure").
 *         - Success/failure status.
 *         - Detailed validation results or errors.
 */
TestResult validateLayerStructure(const pxr::UsdStageRefPtr& stage) {
    if (!stage) {
        return {"Validate Layer Structure", false, "Invalid stage reference."};
    }

    auto layerStack = stage->GetLayerStack();
    if (layerStack.empty()) {
        return {"Validate Layer Structure", false, "Layer stack is empty."};
    }

    std::vector<std::string> errors;
    std::unordered_set<std::string> layerIds;

    const auto& rootLayer = layerStack.front();
    if (rootLayer) {
        if (!rootLayer->IsAnonymous() && !rootLayer->HasDefaultPrim()) {
            errors.push_back("Root layer missing default prim specification: " + 
                             rootLayer->GetIdentifier());
        }
    } else {
        return {"Validate Layer Structure", false, "The first layer in the stack is null."};
    }

    for (size_t i = 0; i < layerStack.size(); ++i) {
        const auto& layer = layerStack[i];
        if (!layer) {
            errors.push_back("Broken reference at layer index " + std::to_string(i));
            continue;
        }

        std::string layerId = layer->GetIdentifier();

        if (layerIds.count(layerId) > 0) {
            errors.push_back("Duplicate layer identifier found: " + layerId);
        } else {
            layerIds.insert(layerId);
        }

        for (const auto& subLayerPath : layer->GetSubLayerPaths()) {
            auto subLayer = pxr::SdfLayer::FindOrOpen(subLayerPath);
            if (!subLayer) {
                errors.push_back("Unresolved sublayer: " + std::string(subLayerPath));
                continue;
            }

            for (const auto& ref : subLayer->GetExternalReferences()) {
                if (!pxr::SdfLayer::FindOrOpen(ref)) {
                    errors.push_back("Broken external reference in sublayer: " + ref);
                }
            }
        }

        auto rootPrimSpec = layer->GetPrimAtPath(pxr::SdfPath("/"));
        if (rootPrimSpec) {

            for (const auto& ref : rootPrimSpec->GetReferenceList().GetAddedOrExplicitItems()) {
                if (!ref.GetAssetPath().empty() && !pxr::SdfLayer::FindOrOpen(ref.GetAssetPath())) {
                    errors.push_back("Broken reference in layer: " + ref.GetAssetPath());
                }
            }

            for (const auto& payload : rootPrimSpec->GetPayloadList().GetAddedOrExplicitItems()) {
                if (!payload.GetAssetPath().empty() && !pxr::SdfLayer::FindOrOpen(payload.GetAssetPath())) {
                    errors.push_back("Broken payload in layer: " + payload.GetAssetPath());
                }
            }
        } else {
            errors.push_back("Layer at index " + std::to_string(i) + 
                             " has no root prim (possibly a library or session layer).");
        }
    }

    if (!errors.empty()) {
        std::string errorMsg = "Layer structure validation failed with the following issues:\n";
        for (const auto& err : errors) {
            errorMsg += "- " + err + "\n";
        }
        return {"Validate Layer Structure", false, errorMsg};
    }

    return {"Validate Layer Structure", true, "Layer stack and all references are valid."};
}

/**
 * @brief Validates the variants and their relationships in a USD file.
 *
 * Ensures that all variant sets and their selections are valid by:
 * - Verifying non-empty variant set names and variant lists.
 * - Testing variant selection to ensure no prim becomes invalid.
 * - Restoring original variant selections after validation.
 *
 * Reports missing variants, invalid selections, or prims that fail after variant changes.
 * Passes if no variants are found unless they are mandatory.
 *
 * @param stage The USD stage to validate.
 * @return TestResult Containing:
 *         - Test name ("Validate Variants").
 *         - Success/failure status.
 *         - Detailed validation results or errors.
 */
TestResult validateVariants(const pxr::UsdStageRefPtr& stage) {
    if (!stage) {
        return {"Validate Variants", false, "Invalid stage reference."};
    }

    std::vector<std::string> errors;
    bool foundAnyVariants = false;

    // Traverse every prim (including inactive and instance proxies) to find variant sets
    for (auto prim : stage->TraverseAll()) {
        if (!prim.IsValid()) {
            errors.push_back("Encountered an invalid prim at: " + prim.GetPath().GetString());
            continue;
        }

        pxr::UsdVariantSets varSets = prim.GetVariantSets();
        std::vector<std::string> setNames;
        varSets.GetNames(&setNames);

        if (!setNames.empty()) {
            foundAnyVariants = true;
        }

        for (const auto& setName : setNames) {
            if (setName.empty()) {
                errors.push_back("Found a variant set with an empty name at: " + prim.GetPath().GetString());
                continue;
            }

            auto varSet = varSets.GetVariantSet(setName);
            std::vector<std::string> variantNames = varSet.GetVariantNames();

            if (variantNames.empty()) {
                errors.push_back("Variant set '" + setName + "' has no variants on prim: " +
                                 prim.GetPath().GetString());
                continue;
            }

            std::string originalSelection = varSet.GetVariantSelection();

            for (const auto& variantName : variantNames) {
                if (variantName.empty()) {
                    errors.push_back("Empty variant name in set '" + setName + "' at: " +
                                     prim.GetPath().GetString());
                    continue;
                }

                if (!varSet.SetVariantSelection(variantName)) {
                    errors.push_back("Failed to set variant '" + variantName +
                                     "' in set '" + setName + "' at: " +
                                     prim.GetPath().GetString());
                    continue;
                }

                auto variantPrim = stage->GetPrimAtPath(prim.GetPath());
                if (!variantPrim.IsValid()) {
                    errors.push_back("Prim became invalid after setting variant '" + variantName +
                                     "' in set '" + setName + "' at: " + prim.GetPath().GetString());
                }
            }

            // Restore the original selection
            if (!originalSelection.empty()) {
                varSet.SetVariantSelection(originalSelection);
            }
        }
    }

    if (!foundAnyVariants) {
        return {
            "Validate Variants",
            true,
            "No variants found in the scene. That's acceptable."
        };
    }

    if (!errors.empty()) {
        std::string errorMsg = "Variant validation failed with the following issues:\n";
        for (const auto& err : errors) {
            errorMsg += "- " + err + "\n";
        }
        return {"Validate Variants", false, errorMsg};
    }

    return {"Validate Variants", true, "All variants and their selections are valid."};
}

/**
 * @brief Displays the usage instructions and available options for the USD test runner program.
 */ 
void displayHelp() {
    std::cout << R"(
Usage: usdTestRunner <path-to-usd-file> [options]

Options:
  -only-geometry    Run only geometry validation
  -only-shaders     Run only shader validation
  -only-layers      Run only layer structure validation
  -skip-geometry    Skip geometry validation
  -skip-shaders     Skip shader validation
  -skip-layers      Skip layer structure validation
  -output <path>    Export results to specified file path
  -help             Display this help message

Note: 
- 'only' flags and 'skip' flags are mutually exclusive
- Multiple 'skip' flags can be combined
- Only one 'only' flag can be used at a time
)";
}

/**
 * @brief Parses command line arguments to determine test configuration
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return TestConfig object with enabled/disabled tests
 */
TestConfig parseArguments(int argc, char* argv[]) {
    TestConfig config;
    std::unordered_set<std::string> args;
    
    // Collect all arguments
    for (int i = 1; i < argc; ++i) {
        args.insert(argv[i]);
        
        // Check for output path
        if (std::string(argv[i]) == "-output" && i + 1 < argc) {
            config.outputPath = argv[i + 1];
            ++i;  // Skip the next argument since it's the path
        }
    }

    // Check for help flag first
    if (args.count("-help")) {
        displayHelp();
        exit(0);
    }

    // Now check for USD file path
    if (argc < 2 || argv[1][0] == '-') {  // First argument missing or is a flag
        displayHelp();
        exit(1);
    }

    // Handle 'only' flags
    if (args.count("-only-geometry")) {
        config.runGeometry = true;
        config.runShaders = false;
        config.runLayers = false;
        config.runVariants = false;
    } else if (args.count("-only-shaders")) {
        config.runGeometry = false;
        config.runShaders = true;
        config.runLayers = false;
        config.runVariants = false;
    } else if (args.count("-only-layers")) {
        config.runGeometry = false;
        config.runShaders = false;
        config.runLayers = true;
        config.runVariants = false;
    } else if (args.count("-only-variants")) {
        config.runGeometry = false;
        config.runShaders = false;
        config.runLayers = false;
        config.runVariants = true;
    } else {
        // Handle 'skip' flags
        if (args.count("-skip-geometry")) config.runGeometry = false;
        if (args.count("-skip-shaders")) config.runShaders = false;
        if (args.count("-skip-layers")) config.runLayers = false;
        if (args.count("-skip-variants")) config.runLayers = false;
    }

    // Validate arguments
    int onlyFlags = args.count("-only-geometry") + args.count("-only-shaders") + 
                   args.count("-only-layers") + args.count("-only-variants");
    
    if (onlyFlags > 1) {
        std::cerr << "Error: Only one '-only' flag can be used at a time.\n";
        displayHelp();
        exit(1);
    }

    // Check for invalid combination of 'only' and 'skip' flags
    if (onlyFlags > 0 && (args.count("-skip-geometry") || args.count("-skip-shaders") || 
                         args.count("-skip-layers") || args.count("-only-variants"))) {
        std::cerr << "Error: Cannot combine '-only' and '-skip' flags.\n";
        displayHelp();
        exit(1);
    }

    if (!config.hasEnabledTests()) {
        std::cerr << "Error: Cannot skip all tests. At least one test must run.\n";
        displayHelp();
        exit(1);
    }

    return config;
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
    // Check for help flag before anything else
    if (argc > 1 && std::string(argv[1]) == "-help") {
        displayHelp();
        return 0;
    }

    // Missing arguments case
    if (argc < 2) {
        displayHelp();
        return 1;
    }

    const std::string usdFilePath = argv[1];
    TestRunner runner(usdFilePath);

    // Add tests with their identifiers
    runner.addTest("geometry", validateGeometry);
    runner.addTest("shaders", validateShaders);
    runner.addTest("layers", validateLayerStructure);
    runner.addTest("variants", validateVariants);

    // Parse command line arguments and run tests
    TestConfig config = parseArguments(argc, argv);
    runner.runTests(config);

    return 0;
}