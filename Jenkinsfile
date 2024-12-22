/**
 * USD Validation Pipeline
 * 
 * This Jenkins pipeline automates the build and testing process for the USD validation tool.
 * It supports both Windows and Linux environments, managing the build process, test execution,
 * and result reporting for USD file validation.
 */

pipeline {
    parameters {
        choice(
            name: 'AGENT',
            choices: ['linux_agent', 'windows_agent'],
            description: 'Choose the build agent (linux_agent or windows_agent)'
        )
        string(
            name: 'BRANCH_NAME',
            defaultValue: '',
            description: 'Branch to build (leave blank for detected branch)'
        )
    }

    agent {
        label "${params.AGENT}"
    }

    // Environment configuration for both Windows and Linux platforms
    environment {
        CMAKE_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Program Files\\CMake\\bin\\cmake.exe' : '/usr/bin/cmake'}"
        PYTHON_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Users\\lwolu\\AppData\\Local\\Programs\\Python\\Python310' : '/usr/bin/python3'}"
        USD_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Users\\lwolu\\OneDrive\\Documents\\Coding\\dev\\usd-automated-testing\\usd' : '/usr/local/USD'}"
        GIT_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Program Files\\Git\\bin\\git.exe' : '/usr/bin/git'}"
        PATH = """${params.AGENT == 'windows_agent' \
            ? env.PYTHON_HOME + ';' + env.PYTHON_HOME + '\\Scripts;C:\\Windows\\System32;' + env.PATH \
            : env.PATH}"""
    }

    stages {
        // Validate build configuration and branch selection
        stage('Determine Branch and Agent') {
            steps {
                script {
                    if (params.AGENT != 'linux_agent' && params.AGENT != 'windows_agent') {
                        error "Unknown or misconfigured agent: ${params.AGENT}"
                    }
                    
                    def actualBranch = params.BRANCH_NAME ?: env.BRANCH_NAME
                    if (!actualBranch) {
                        error "Branch name could not be determined. Provide a BRANCH_NAME parameter."
                    }
                }
            }
        }

        // Source code checkout stage
        stage('Checkout') {
            steps {
                script {
                    def actualBranch = params.BRANCH_NAME ?: env.BRANCH_NAME
                    checkout([$class: 'GitSCM',
                              branches: [[name: "*/${actualBranch}"]],
                              userRemoteConfigs: [[url: 'https://github.com/lwoluke/usd-automated-testing']]
                    ])
                }
            }
        }

        // Validate required tools and environment
        stage('Environment Validation') {
            steps {
                script {
                    if (params.AGENT == 'windows_agent') {
                        bat "\"${CMAKE_HOME}\" --version"
                        bat "\"${PYTHON_HOME}\\python.exe\" --version"
                        bat "\"${GIT_HOME}\" --version"
                        bat """
                        if not exist "${USD_HOME}" (
                            echo USD_HOME directory does not exist!
                            exit /b 1
                        )
                        """
                    } else {
                        sh "${CMAKE_HOME} --version"
                        sh "${PYTHON_HOME} --version"
                        sh "${GIT_HOME} --version"
                        sh """
                        if [ ! -d "${USD_HOME}" ]; then
                            echo "USD_HOME directory does not exist!"
                            exit 1
                        fi
                        """
                    }
                }
            }
        }

        // Build stage using CMake
        stage('Build') {
            steps {
                script {
                    if (params.AGENT == 'windows_agent') {
                        bat """
                        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat"
                        "${CMAKE_HOME}" -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSD_ROOT=%USD_HOME%
                        "${CMAKE_HOME}" --build build --config Release
                        """
                    } else {
                        sh """
                        mkdir -p build && cd build
                        ${CMAKE_HOME} -DUSD_ROOT="${USD_HOME}" ..
                        make -j\$(nproc)
                        """
                    }
                }
            }
        }

        // Execute USD validation tests
        stage('Test') {
            steps {
                script {
                    def outputBaseDir = "test_results"
                    
                    if (params.AGENT == 'windows_agent') {
                        bat "mkdir \"${outputBaseDir}\" 2>nul"
                    } else {
                        sh "mkdir -p ${outputBaseDir}"
                    }

                    def testFiles = findFiles(glob: 'test/*/*.usda')
                    if (testFiles.length == 0) {
                        error "No test files found. Check the test directory structure."
                    }

                    // Generate and execute test commands for each platform
                    if (params.AGENT == 'windows_agent') {
                        def testCommands = ""
                        def comparisonCommands = ""

                        testFiles.each { fileWrapper ->
                            def normalizedPath = fileWrapper.path.replace('\\', '/')
                            def subDir = normalizedPath.replaceFirst('test/', '').split('/')[0]
                            def testFileName = normalizedPath.split('/').last().replace('.usda', '')

                            testCommands += "if not exist \"${outputBaseDir}\\${subDir}\" mkdir \"${outputBaseDir}\\${subDir}\"\n"
                            testCommands += "\".\\build\\usdTestRunner.exe\" \"${normalizedPath}\" > \"${outputBaseDir}\\${subDir}\\${testFileName}.txt\"\n"
                            comparisonCommands += "fc /W \"test\\${subDir}\\${testFileName}_expected.txt\" \"${outputBaseDir}\\${subDir}\\${testFileName}.txt\"\n"
                        }

                        writeFile file: 'run_tests.bat', text: testCommands
                        writeFile file: 'compare_results.bat', text: comparisonCommands

                        bat "run_tests.bat"
                        bat "compare_results.bat"
                    } else {
                        testFiles.each { fileWrapper ->
                            def normalizedPath = fileWrapper.path.replace('\\', '/')
                            def subDir = normalizedPath.replaceFirst('test/', '').split('/')[0]
                            def testFileName = normalizedPath.split('/').last().replace('.usda', '')

                            sh """
                            mkdir -p ${outputBaseDir}/${subDir}
                            ./build/usdTestRunner "${normalizedPath}" > "${outputBaseDir}/${subDir}/${testFileName}.txt"
                            diff -w -B "test/${subDir}/${testFileName}_expected.txt" "${outputBaseDir}/${subDir}/${testFileName}.txt"
                            """
                        }
                    }
                }
            }
        }

        // Generate HTML test report
        stage('Report') {
            steps {
                script {
                    if (params.AGENT == 'windows_agent') {
                        bat '''
                        mkdir reports
                        echo ^<html^>^<head^>^<title^>USD Validation Report^</title^>^</head^>^<body^>^<h1^>USD Validation Test Results^</h1^>^<ul^> > reports\\index.html
                        for /R test_results %%f in (*.txt) do (
                            echo ^<li^>^<a href="%%f"^>%%f^</a^>^</li^> >> reports\\index.html
                        )
                        echo ^</ul^>^</body^>^</html^> >> reports\\index.html
                        '''
                    } else {
                        sh '''
                        mkdir -p reports
                        echo "<html><head><title>USD Validation Report</title></head><body><h1>USD Validation Test Results</h1><ul>" > reports/index.html
                        for f in test_results/**/*.txt; do
                          echo "<li><a href=\"../$f\">$f</a></li>" >> reports/index.html
                        done
                        echo "</ul></body></html>" >> reports/index.html
                        '''
                    }

                    publishHTML(target: [
                        allowMissing: false,
                        alwaysLinkToLastBuild: true,
                        keepAll: true,
                        reportDir: 'reports',
                        reportFiles: 'index.html',
                        reportName: 'Validation Report'
                    ])
                }
            }
        }
    }

    // Post-build actions
    post {
        always {
            archiveArtifacts artifacts: '**/test_results/**/*.txt', fingerprint: true
        }
        failure {
            echo 'Tests failed. Check the report for details.'
        }
    }
}