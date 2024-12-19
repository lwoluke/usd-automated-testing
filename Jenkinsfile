pipeline {
    parameters {
        choice(name: 'AGENT', choices: ['linux', 'windows'], description: 'Choose the build agent (linux or windows)')
        string(name: 'BRANCH_NAME', defaultValue: '', description: 'Branch to build (leave blank for detected branch)')
    }
    agent { 
        label "${params.AGENT?.toLowerCase()?.trim() ?: env.NODE_NAME?.toLowerCase()?.trim()}" 
    }
    environment {
        CMAKE_HOME = "${env.NODE_NAME?.trim()?.toLowerCase() == 'windows' ? 'C:\\Program Files\\CMake\\bin\\cmake.exe' : '/usr/bin/cmake'}"
        PYTHON_HOME = "${env.NODE_NAME?.trim()?.toLowerCase() == 'windows' ? 'C:\\Users\\lwolu\\AppData\\Local\\Programs\\Python\\Python310' : '/usr/bin/python3'}"
        USD_HOME = "${env.NODE_NAME?.trim()?.toLowerCase() == 'windows' ? 'C:\\Users\\lwolu\\OneDrive\\Documents\\Coding\\dev\\usd-automated-testing\\usd' : '/mnt/c/Users/lwolu/OneDrive/Documents/Coding/dev/usd-automated-testing/usd'}"
        GIT_HOME = "${env.NODE_NAME?.trim()?.toLowerCase() == 'windows' ? 'C:\\Program Files\\Git\\bin\\git.exe' : '/usr/bin/git'}"
    }
    stages {
        stage('Determine Branch and Agent') {
            steps {
                script {
                    echo "Parameters:"
                    echo "  AGENT: ${params.AGENT}"
                    echo "Environment:"
                    echo "  NODE_NAME: ${env.NODE_NAME}"

                    def actualBranch = params.BRANCH_NAME ?: env.BRANCH_NAME
                    if (!actualBranch) {
                        error "Branch name could not be determined. Ensure the pipeline is triggered by a GitHub webhook or provide a BRANCH_NAME parameter."
                    }

                    def actualAgent = params.AGENT?.toLowerCase()?.trim() ?: env.NODE_NAME?.toLowerCase()?.trim()
                    echo "  Resolved Agent: ${actualAgent}"

                    if (!actualAgent || (actualAgent != 'linux' && actualAgent != 'windows')) {
                        error "Unknown or misconfigured agent: ${actualAgent}"
                    }

                    echo "Building branch: ${actualBranch}"
                    echo "Running on agent: ${actualAgent}"
                }
            }
        }
        stage('Checkout') {
            steps {
                script {
                    def actualBranch = params.BRANCH_NAME ?: env.BRANCH_NAME
                    checkout([$class: 'GitSCM',
                              branches: [[name: "*/${actualBranch}"]],
                              userRemoteConfigs: [[url: 'https://github.com/lwoluke/usd-automated-testing']]])
                }
            }
        }
        stage('Check Tools') {
            steps {
                script {
                    def normalizedNodeName = env.NODE_NAME?.trim()?.toLowerCase()
                    if (normalizedNodeName == 'windows') {
                        echo "Running on Windows"
                        bat "\"${CMAKE_HOME}\" --version"
                        bat "\"${PYTHON_HOME}\\python.exe\" --version"
                        bat "\"${GIT_HOME}\" --version"
                        echo "USD_HOME is set to ${USD_HOME}"
                    } else if (normalizedNodeName == 'linux') {
                        echo "Running on Linux"
                        sh "${CMAKE_HOME} --version"
                        sh "${PYTHON_HOME} --version"
                        sh "${GIT_HOME} --version"
                        echo "USD_HOME is set to ${USD_HOME}"
                    } else {
                        error "Unknown agent: ${env.NODE_NAME}"
                    }
                }
            }
        }
        stage('Verify Environment') {
            steps {
                script {
                    echo "PATH environment variable: ${env.PATH}"
                    echo "CMAKE_HOME: ${CMAKE_HOME}"
                    echo "PYTHON_HOME: ${PYTHON_HOME}"
                    echo "USD_HOME: ${USD_HOME}"
                    echo "GIT_HOME: ${GIT_HOME}"
                }
            }
        }
    }
}
