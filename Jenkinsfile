pipeline {
    parameters {
        choice(name: 'AGENT', choices: ['linux', 'windows'], description: 'Choose the agent to run the pipeline', defaultValue: 'linux')
    }
    agent { label "${['linux', 'windows'].contains(params.AGENT) ? params.AGENT : 'linux'}" }
    environment {
        CMAKE_HOME = "${params.AGENT == 'linux' ? '/usr/bin/cmake' : 'C:\\Program Files\\CMake\\bin\\cmake.exe'}"
        PYTHON_HOME = "${params.AGENT == 'linux' ? '/usr/bin/python3' : 'C:\\Python39'}"
        USD_HOME = "${params.AGENT == 'linux' ? '/usr/local/usd' : 'C:\\USD'}"
    }
    stages {
        stage('Check Branch and Agent') {
            steps {
                echo "Running on branch: ${BRANCH_NAME}"
                echo "Running on agent: ${env.NODE_NAME}"
            }
        }
        stage('Environment Info') {
            steps {
                echo "CMAKE_HOME: ${CMAKE_HOME}"
                echo "PYTHON_HOME: ${PYTHON_HOME}"
                echo "USD_HOME: ${USD_HOME}"
            }
        }
        stage('Check Tools') {
            steps {
                script {
                    try {
                        if (params.AGENT == 'linux') {
                            sh "${CMAKE_HOME} --version"
                            sh "${PYTHON_HOME} --version"
                        } else if (params.AGENT == 'windows') {
                            bat "%CMAKE_HOME% --version"
                            bat "%PYTHON_HOME% --version"
                        }
                    } catch (Exception e) {
                        error "Tool check failed: ${e.message}"
                    }
                }
            }
        }
    }
}
