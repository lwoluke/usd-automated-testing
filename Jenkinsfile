pipeline {
    agent any
    stages {
        stage('Check Tools on Linux') {
            agent { label 'linux' }
            environment {
                CMAKE_HOME = '/usr/bin/cmake'
                PYTHON_HOME = '/usr/bin/python3'
                USD_HOME = '/usr/local/usd'
            }
            steps {
                sh '${CMAKE_HOME} --version'
                sh '${PYTHON_HOME} --version'
                echo "USD_HOME is set to ${USD_HOME}"
            }
        }
        stage('Check Tools on Windows') {
            agent { label 'windows' }
            environment {
                CMAKE_HOME = 'C:\\Program Files\\CMake\\bin\\cmake.exe'
                PYTHON_HOME = 'C:\\Python39'
                USD_HOME = 'C:\\USD'
            }
            steps {
                bat '%CMAKE_HOME% --version'
                bat '%PYTHON_HOME% --version'
                echo "USD_HOME is set to %USD_HOME%"
            }
        }
    }
}
