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
        CMAKE_HOME    = "${params.AGENT == 'windows_agent' ? 'C:\\Program Files\\CMake\\bin\\cmake.exe' : '/usr/bin/cmake'}"
        PYTHON_HOME   = "${params.AGENT == 'windows_agent' ? 'C:\\Users\\lwolu\\AppData\\Local\\Programs\\Python\\Python310' : '/usr/bin/python3'}"
        USD_HOME      = "${params.AGENT == 'windows_agent' ? 'C:\\Users\\lwolu\\OneDrive\\Documents\\Coding\\dev\\usd-automated-testing\\usd' : '/usr/local/USD'}"
        GIT_HOME      = "${params.AGENT == 'windows_agent' ? 'C:\\Program Files\\Git\\bin\\git.exe' : '/usr/bin/git'}"
        
        // USD test files to be run
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
                        error "Branch name could not be determined. Ensure the pipeline is triggered by a GitHub webhook or provide a BRANCH_NAME parameter."
                    }
                    echo "Building branch: ${actualBranch}"
                }
            }
        }

        stage('Set OS-specific Environment') {
            steps {
                script {
                    if (params.AGENT == 'windows_agent') {
                        env.COMSPEC = "C:\\Windows\\System32\\cmd.exe"
                        env.PATH = "C:\\Windows\\System32;C:\\Windows;C:\\Windows\\System32\\Wbem;C:\\Windows\\System32\\WindowsPowerShell\\v1.0"
                    }
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
                        bat 'echo %PATH%'
                        bat 'echo %COMSPEC%'
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
                    } else if (params.AGENT == 'linux_agent') {
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
                    } else {
                        error "Unknown agent: ${params.AGENT}"
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
                        // Windows build steps
                        bat "\"${CMAKE_HOME}\" -S . -B build -DUSD_ROOT=%USD_HOME%"
                        bat "\"${CMAKE_HOME}\" --build build --config Release"
                    } else {
                        // Linux build steps
                        sh "mkdir -p build && cd build && ${CMAKE_HOME} -DUSD_ROOT=${env.USD_HOME} .. && make -j\$(nproc)"
                    }
                }
            }
        }

        stage('Test') {
            steps {
                script {
                    // Run tests on each USD file and store the results
                    def testCommands = ""
                    TEST_FILES.split(' ').each { file ->
                        def testFilePath = "test/${file}"
                        def outputFile = "test_results_${file.replace('.usda','')}.json"
                        if (params.AGENT == 'windows_agent') {
                            testCommands += "\".\\build\\usdTestRunner\" \"${testFilePath}\" > \"${outputFile}\" \n"
                        } else {
                            testCommands += "./build/usdTestRunner ${testFilePath} > ${outputFile}\n"
                        }
                    }

                    if (params.AGENT == 'windows_agent') {
                        writeFile file: 'run_tests.bat', text: testCommands
                        bat 'run_tests.bat'
                    } else {
                        sh testCommands
                    }
                }
            }
        }

        stage('Report') {
            steps {
                script {
                    // Generate a simple HTML summary report from the JSON test results
                    sh '''
                    mkdir -p reports
                    echo "<html><head><title>USD Validation Report</title></head><body><h1>USD Validation Test Results</h1><ul>" > reports/index.html
                    for f in test_results_*.json; do
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
    post {
        always {
            // Archive all test result files
            archiveArtifacts artifacts: '**/test_results_*.json', fingerprint: true
        }
        failure {
            echo 'Tests failed. Check the report for details.'
        }
    }
}
