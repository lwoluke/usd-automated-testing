pipeline {
    parameters {
        choice(name: 'AGENT', choices: ['linux', 'windows'], description: 'Choose the build agent (linux or windows)')
        string(name: 'BRANCH_NAME', defaultValue: '', description: 'Branch to build (leave blank for detected branch)')
    }
    agent { 
        label "${params.AGENT ?: env.NODE_NAME}" 
    }
    environment {
        CMAKE_HOME = "${env.NODE_NAME == 'windows' ? 'C:\\Program Files\\CMake\\bin\\cmake.exe' : '/usr/bin/cmake'}"
        PYTHON_HOME = "${env.NODE_NAME == 'windows' ? 'C:\\Python39' : '/usr/bin/python3'}"
        USD_HOME = "${env.NODE_NAME == 'windows' ? 'C:\\USD' : '/usr/local/usd'}"
    }
    stages {
        stage('Determine Branch and Agent') {
            steps {
                script {
                    def actualBranch = params.BRANCH_NAME ?: env.BRANCH_NAME
                    if (!actualBranch) {
                        error "Branch name could not be determined. Ensure the pipeline is triggered by a GitHub webhook or provide a BRANCH_NAME parameter."
                    }
                    def actualAgent = params.AGENT ?: env.NODE_NAME
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
                    if (env.NODE_NAME == 'windows') {
                        echo "Running on Windows"
                        bat "\"${CMAKE_HOME}\" --version"
                        bat "\"${PYTHON_HOME}\" --version"
                        echo "USD_HOME is set to ${USD_HOME}"
                    } else {
                        echo "Running on Linux"
                        sh "${CMAKE_HOME} --version"
                        sh "${PYTHON_HOME} --version"
                        echo "USD_HOME is set to ${USD_HOME}"
                    }
                }
            }
        }
    }
}
