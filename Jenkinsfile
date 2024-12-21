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
    environment {
        CMAKE_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Program Files\\CMake\\bin\\cmake.exe' : '/usr/bin/cmake'}"
        PYTHON_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Users\\lwolu\\AppData\\Local\\Programs\\Python\\Python310' : '/usr/bin/python3'}"
        USD_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Users\\lwolu\\OneDrive\\Documents\\Coding\\dev\\usd-automated-testing\\usd' : '/usr/local/USD'}"
        GIT_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Program Files\\Git\\bin\\git.exe' : '/usr/bin/git'}"

        // Build PATH depending on the agent
        PATH = """${params.AGENT == 'windows_agent' \
            ? env.PYTHON_HOME + ';' + env.PYTHON_HOME + '\\Scripts;C:\\Windows\\System32;' + env.PATH \
            : env.PATH}"""
        
        TEST_FILES = "all_pass.usda empty.usda invalid_layer.usda no_shaders.usda"
    }

    stages {
        stage('Determine Branch and Agent') {
            steps {
                script {
                    echo "Parameters:"
                    echo "  AGENT: ${params.AGENT}"
                    echo "Environment:"
                    echo "  NODE_NAME: ${env.NODE_NAME}"
                    echo "Resolved Agent: ${params.AGENT}"

                    if (params.AGENT != 'linux_agent' && params.AGENT != 'windows_agent') {
                        error "Unknown or misconfigured agent: ${params.AGENT}"
                    }

                    echo "Running on agent: ${params.AGENT}"

                    def actualBranch = params.BRANCH_NAME ?: env.BRANCH_NAME
                    if (!actualBranch) {
                        error "Branch name could not be determined. Provide a BRANCH_NAME parameter."
                    }
                    echo "Building branch: ${actualBranch}"
                }
            }
        }

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

        stage('Environment Validation') {
            steps {
                script {
                    echo "Running on ${params.AGENT}"

                    if (params.AGENT == 'windows_agent') {
                        bat 'echo Debugging Environment...'
                        bat 'where cmd'
                        bat 'dir'
                        bat 'echo Debugging PATH: %PATH%'
                        bat 'echo Debugging COMSPEC: %COMSPEC%'
                        bat 'set'

                        echo "Validating Tools..."
                        bat "\"${CMAKE_HOME}\" --version"
                        bat "\"${PYTHON_HOME}\\python.exe\" --version"
                        bat "\"${GIT_HOME}\" --version"

                        echo "Validating USD_HOME..."
                        bat """
                        if not exist "${USD_HOME}" (
                            echo USD_HOME directory does not exist!
                            exit /b 1
                        )
                        dir "${USD_HOME}" | findstr /c:"File(s)" >nul
                        if errorlevel 1 (
                            echo USD_HOME directory is empty!
                            exit /b 1
                        )
                        """
                    } else {
                        sh 'echo Debugging Environment...'
                        sh 'env'

                        echo "Validating Tools..."
                        sh "${CMAKE_HOME} --version"
                        sh "${PYTHON_HOME} --version"
                        sh "${GIT_HOME} --version"

                        echo "Validating USD_HOME..."
                        sh '''
                        if [ ! -d "${USD_HOME}" ]; then
                            echo "USD_HOME directory does not exist!"
                            exit 1
                        fi
                        if [ -z "$(ls -A ${USD_HOME})" ]; then
                            echo "USD_HOME directory is empty!"
                            exit 1
                        fi
                        '''
                    }

                    echo "Environment Variables:"
                    echo "PATH: ${env.PATH}"
                    echo "CMAKE_HOME: ${CMAKE_HOME}"
                    echo "PYTHON_HOME: ${PYTHON_HOME}"
                    echo "USD_HOME: ${USD_HOME}"
                    echo "GIT_HOME: ${GIT_HOME}"
                }
            }
        }

        stage('Build') {
            steps {
                script {
                    if (params.AGENT == 'windows_agent') {
                        echo "Running on Windows agent..."
                        bat """
                        echo Activating Visual Studio environment...
                        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat"

                        echo Running CMake configure step...
                        "${CMAKE_HOME}" -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSD_ROOT=%USD_HOME%
                        if %errorlevel% neq 0 (
                            echo CMake configure step failed.
                            exit /b 1
                        )

                        echo Running CMake build step...
                        "${CMAKE_HOME}" --build build --config Release
                        if %errorlevel% neq 0 (
                            echo CMake build step failed.
                            exit /b 1
                        )

                        echo Listing contents of build directory...
                        dir build

                        echo Checking for usdTestRunner.exe:
                        dir build\\usdTestRunner.exe
                        """
                    } else {
                        echo "Running on Linux agent..."
                        sh """
                        mkdir -p build && cd build
                        ${CMAKE_HOME} -DUSD_ROOT="${USD_HOME}" ..
                        if [ \$? -ne 0 ]; then
                            echo "CMake configure step failed."
                            exit 1
                        fi

                        make -j\$(nproc)
                        if [ \$? -ne 0 ]; then
                            echo "Make build step failed."
                            exit 1
                        fi
                        """
                    }
                }
            }
        }

        stage('Test') {
            steps {
                script {
                    def outputBaseDir = "test_results"
                    
                    if (params.AGENT == 'windows_agent') {
                        // Windows test logic
                        bat "mkdir \"${outputBaseDir}\" 2>nul"

                        // Find test files with Jenkins findFiles
                        def testFiles = findFiles(glob: 'test/*/*.usda')
                        echo "Found test files: ${testFiles.collect { it.path }}"

                        if (testFiles.length == 0) {  // Changed from isEmpty() to length == 0
                            error "No test files found. Check the test directory and glob pattern."
                        }

                        def testCommands = ""
                        def comparisonCommands = ""

                        testFiles.each { fileWrapper ->
                            def normalizedPath = fileWrapper.path.replace('\\', '/')
                            def subDir = normalizedPath.replaceFirst('test/', '').split('/')[0]
                            def testFileName = normalizedPath.split('/').last().replace('.usda', '')

                            // On Windows we have usdTestRunner.exe in build dir
                            // We'll store results directly under test_results
                            testCommands += "if not exist \"${outputBaseDir}\\${subDir}\" mkdir \"${outputBaseDir}\\${subDir}\"\n"
                            testCommands += "\".\\build\\usdTestRunner.exe\" \"${normalizedPath}\" > \"${outputBaseDir}\\${subDir}\\${testFileName}.txt\"\n"

                            comparisonCommands += "fc /W \"test\\${subDir}\\${testFileName}_expected.txt\" \"${outputBaseDir}\\${subDir}\\${testFileName}.txt\"\n"
                        }

                        echo "Test Commands:\n${testCommands}"
                        echo "Comparison Commands:\n${comparisonCommands}"

                        writeFile file: 'run_tests.bat', text: testCommands
                        writeFile file: 'compare_results.bat', text: comparisonCommands

                        bat 'echo Contents of run_tests.bat:'
                        bat 'type run_tests.bat'
                        bat 'echo Contents of compare_results.bat:'
                        bat 'type compare_results.bat'

                        bat """
                        echo Running tests...
                        run_tests.bat > test_run.log 2>&1
                        if errorlevel 1 (
                            echo Tests failed. Check test_run.log for details.
                            type test_run.log
                            exit /b 1
                        )
                        """

                        bat """
                        echo Comparing test results...
                        compare_results.bat > compare_results.log 2>&1
                        if errorlevel 1 (
                            echo Comparison failed. Check compare_results.log for details.
                            type compare_results.log
                            exit /b 1
                        )
                        """
                    } else {
                        // Linux test logic
                        sh "mkdir -p ${outputBaseDir}"

                        def testFiles = findFiles(glob: 'test/*/*.usda')
                        echo "Found test files: ${testFiles.collect { it.path }}"

                        if (testFiles.length == 0) {  // Changed from isEmpty() to length == 0
                            error "No test files found on Linux agent."
                        }

                        // On Linux, run ./build/usdTestRunner and diff
                        def testCommands = ""
                        def comparisonCommands = ""

                        testFiles.each { fileWrapper ->
                            def normalizedPath = fileWrapper.path.replace('\\', '/')
                            def subDir = normalizedPath.replaceFirst('test/', '').split('/')[0]
                            def testFileName = normalizedPath.split('/').last().replace('.usda', '')

                            sh "mkdir -p ${outputBaseDir}/${subDir}"

                            testCommands += "./build/usdTestRunner \"${normalizedPath}\" > \"${outputBaseDir}/${subDir}/${testFileName}.txt\"\n"
                            comparisonCommands += "diff -w -B \"test/${subDir}/${testFileName}_expected.txt\" \"${outputBaseDir}/${subDir}/${testFileName}.txt\"\n"
                        }

                        echo "Test Commands:\n${testCommands}"
                        echo "Comparison Commands:\n${comparisonCommands}"

                        // Run test commands
                        try {
                            sh testCommands
                            sh comparisonCommands
                        } catch (Exception e) {
                            error "Test or comparison failed on Linux. Check logs for details."
                        }
                    }
                }
            }
        }

        stage('Report') {
            when {
                anyOf {
                    expression { params.AGENT == 'windows_agent' }
                    expression { params.AGENT == 'linux_agent' }
                }
            }
            steps {
                script {
                    if (params.AGENT == 'windows_agent') {
                        // Windows: Use bat to generate HTML
                        bat '''
                        mkdir reports
                        echo ^<html^>^<head^>^<title^>USD Validation Report^</title^>^</head^>^<body^>^<h1^>USD Validation Test Results^</h1^>^<ul^> > reports\\index.html
                        for /R test_results %%f in (*.txt) do (
                            echo ^<li^>^<a href="%%f"^>%%f^</a^>^</li^> >> reports\\index.html
                        )
                        echo ^</ul^>^</body^>^</html^> >> reports\\index.html
                        '''
                    } else {
                        // Linux: Use sh to generate HTML
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

    post {
        always {
            archiveArtifacts artifacts: '**/test_results/**/*.txt', fingerprint: true
        }
        failure {
            echo 'Tests failed. Check the report for details.'
        }
    }
}