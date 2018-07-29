// @Library('AL@develop') _

def gitHubRepo = "https://github.al.com.au/rnd/AL_USDMaya.git"

// The list of packages that will be executed, the mode will determine if the stages can be executed in parallel or in serial.
def packages = ['.']

// the root folder where the package will be built
def rootFolder = ""

// write here the name of the jenkins jobs that we want to chain to this one
// def dependentJobs = ["dependentTest1","dependent test 2"],
// you can define dependent jobs either as a list of simple strings (the dependent job) or
// or, if you need to override some parameters of the dependent job you can pass a list like
// [ 'jobname', 'JOBPARM1=value;JOBPARM2=value2']
// so in the end it looks like
//def dependentJobs = [["dependentTest1",'JOBPARM1=value;JOBPARM2=value2'] ,
//                    "dependent test 2"],
def dependentJobs = []

// String, whitespace separated
// Do not use upstream artifacts for now as "develop" artifacts of AL_USD are not compatible
// def upstreamJobs = "AL_USDSchemas"
def upstreamJobs = ""

// flags passed to the rez build -- -- all_tests
def rezBuildOptions = "-i -- -- -j8"

def testingParams = new al.TestingParameters()
testingParams.gitHubRepo = gitHubRepo
testingParams.packagesList = packages
testingParams.dependentJobs = dependentJobs
testingParams.upstreamJobs = upstreamJobs
testingParams.triggerDownstreamJobs = false
testingParams.rootFolder = rootFolder
testingParams.buildOptions = rezBuildOptions
testingParams.testTargetName = "all_tests"
testingParams.createBuildArtifacts = true

timeout(time: 45)
{
    try {

        // Standard build - maya-2017
        node ('CentOS&&Sydney&&!restricted&&!testbed')
        {
            ansiColor('xterm')
            {
                testingParams.buildOptions = "-i --variants 0 -- -- -j8"
                testingParams.testOptions = "--variants 0 -- --"
                testing.runRepositoryTests(testingParams)
                if(currentBuild.result in ["ERROR", "FAILURE"])
                {
                    error "Tests failed"
                }
            }
        } // node

        if(env.BRANCH_NAME == "develop")
        {
            // Standard build - maya-2018/2019
            node ('dot||jukebox||curtain')
            {
                ansiColor('xterm')
                {
                    testingParams.buildOptions = "-i --variants 1 2 -- -- -j8"
                    testingParams.testOptions = "--variants 1 2 -- --"
                    testing.runRepositoryTests(testingParams)
                    if(currentBuild.result in ["ERROR", "FAILURE"])
                    {
                        error "Tests failed"
                    }
                }
            } // node

            // Docker build
            node ('docker')
            {
                checkout scm

                // Sets the status as 'PENDING'
                algit.reportStatusToGitHub('PENDING', 'Docker build pending', "Docker_build_and_tests")

                try {
                    ansiColor('xterm')
                    {
                        def workspace = pwd() + "/src"
                        stage("Opensource Maya2017")
                        {
                            sh "sudo docker run --rm -e \"BUILD_PROCS=8\" -v $workspace:/tmp/usd-build/AL_USDMaya curtain:5000/usd-docker/usd:0.8.5-centos6-maya2017 bash /tmp/usd-build/AL_USDMaya/docker/build_alusdmaya.sh"
                        }

                        algit.reportStatusToGitHub('SUCCESS', 'Docker build success', "Docker_build_and_tests")
                    }
                }
                catch(Exception e) {
                    currentBuild.result = 'UNSTABLE'
                    algit.reportStatusToGitHub(currentBuild.result, 'Docker build error', "Docker_build_and_tests")
                    throw e
                }
                finally {
                    cleanWs notFailBuild: true
                }
            } // node
            
            // Windows build
            node ('ferry')
            {
                checkout scm

                // Sets the status as 'PENDING'
                algit.reportStatusToGitHub('PENDING', 'Windows build pending', "Windows_build")

                try {
                    ansiColor('xterm')
                    {
                        stage("Windows build")
                        {
                            bat "if not exist T: (net use T: \\\\al.com.au\\dfs)"
                            bat "build_scripts\\windows_build.bat"
                            algit.reportStatusToGitHub('SUCCESS', 'Windows build success', "Windows_build")
                        }
                    }
                }
                catch(Exception e) {
                    currentBuild.result = 'UNSTABLE'
                    algit.reportStatusToGitHub(currentBuild.result, 'Windows build error', "Windows_build")
                    throw e
                }
                finally {
                    cleanWs notFailBuild: true
                }
            } // node
        } // if(env.BRANCH_NAME == "develop")

    } // try
    catch (Exception e) {
        currentBuild.result = "FAILURE"
        global.notifyResult(currentBuild.result,
                            'HipChat-JenkinsUsdBuilds-Token',
                            'JenkinsUsdBuilds',
                            '')
        throw e
    }
    finally {

    } // finally

} // End timeout

