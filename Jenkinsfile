pipeline {
    parameters {
        choice(name: 'AGENT', 
               choices: ['linux_agent', 'windows_agent'], 
               description: 'Choose the build agent (linux_agent or windows_agent)')
        string(name: 'BRANCH_NAME', 
               defaultValue: '', 
               description: 'Branch to build (leave blank for detected branch)')
    }
    agent { 
        label "${params.AGENT}"
    }
    environment {
        CMAKE_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Program Files\\CMake\\bin\\cmake.exe' : '/usr/bin/cmake'}"
        PYTHON_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Users\\lwolu\\AppData\\Local\\Programs\\Python\\Python310' : '/usr/bin/python3'}"
        USD_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Users\\lwolu\\OneDrive\\Documents\\Coding\\dev\\usd-automated-testing\\usd' : '/mnt/c/Users/lwolu/OneDrive/Documents/Coding/dev/usd-automated-testing/usd'}"
        GIT_HOME = "${params.AGENT == 'windows_agent' ? 'C:\\Program Files\\Git\\bin\\git.exe' : '/usr/bin/git'}"
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
        stage('Debug PATH') {
            steps {
                bat 'echo %PATH%'
            }
        }
        stage('Check Tools') {
            steps {
                script {
                    if (params.AGENT == 'windows_agent') {
                        echo "Running on Windows"
                        bat "\"${CMAKE_HOME}\" --version"
                        bat "\"${PYTHON_HOME}\\python.exe\" --version"
                        bat "\"${GIT_HOME}\" --version"
                        echo "USD_HOME is set to ${USD_HOME}"
                    } else if (params.AGENT == 'linux_agent') {
                        echo "Running on Linux"
                        sh "${CMAKE_HOME} --version"
                        sh "${PYTHON_HOME} --version"
                        sh "${GIT_HOME} --version"
                        echo "USD_HOME is set to ${USD_HOME}"
                    } else {
                        error "Unknown agent: ${params.AGENT}"
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
