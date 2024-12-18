pipeline {
    parameters {
        choice(name: 'AGENT', choices: ['linux', 'windows'], description: 'Choose the build agent to run the Jenkins pipeline')
    }
    agent { label "${params.AGENT}" }
    environment {
        CMAKE_HOME = "${params.AGENT == 'linux' ? '/usr/bin/cmake' : 'C:\\Program Files\\CMake\\bin\\cmake.exe'}"
        PYTHON_HOME = "${params.AGENT == 'linux' ? '/usr/bin/python3' : 'C:\\Python39'}"
        USD_HOME = "${params.AGENT == 'linux' ? '/usr/local/usd' : 'C:\\USD'}"
    }
    stages {
        stage('Check Tools') {
            steps {
                script {
                    if (params.AGENT == 'linux') {
                        sh "${CMAKE_HOME} --version"
                        sh "${PYTHON_HOME} --version"
                        echo "USD_HOME is set to ${USD_HOME}"
                    } else if (params.AGENT == 'windows') {
                        bat "\"%CMAKE_HOME%\" --version"
                        bat "%PYTHON_HOME% --version"
                        echo "USD_HOME is set to %USD_HOME%"
                    }
                }
            }
        }
    }
}
