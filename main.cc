#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "globals.hh"
#include "G4UImanager.hh"
#include "Randomize.hh"

#include "G4GenericPhysicsList.hh"

#include "DicomDetector.hh"

#include "DicomActionInitialization.hh"

#include "DicomHandler.hh"

#include "QGSP_BIC.hh"
#include "G4tgrMessenger.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif

int main(int argc,char** argv)
{

    new G4tgrMessenger;

    CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);
    CLHEP::HepRandom::setTheSeed(24534575684783);
    long seeds[2];
    seeds[0] = 534524575674523;
    seeds[1] = 526345623452457;
    CLHEP::HepRandom::setTheSeeds(seeds);

    // Construct the default run manager
    char* nthread_c = getenv("DICOM_NTHREADS");

    unsigned nthreads = 4;
    unsigned env_threads = 0;

    if (nthread_c) {
        env_threads = G4UIcommand::ConvertToDouble(nthread_c);
    }
    if(env_threads > 0) {
        nthreads = env_threads;
    }

    G4MTRunManager* runManager = new G4MTRunManager;
    runManager->SetNumberOfThreads(nthreads);

    G4cout << "\n\n\tDICOM running in multithreaded mode with " << nthreads
           << " threads\n\n" << std::endl;

    // Treatment of DICOM images before creating the G4runManager
    DicomHandler* dcmHandler = new DicomHandler;
    dcmHandler->CheckFileFormat();

    // Initialisation of physics, geometry, primary particles ...
    DicomDetectorConstruction* theGeometry = new DicomDetectorConstruction();
    runManager->SetUserInitialization(theGeometry);

    std::vector<G4String>* MyConstr = new std::vector<G4String>;
    MyConstr->push_back("G4EmStandardPhysics");
    G4VModularPhysicsList* phys = new G4GenericPhysicsList(MyConstr);
    runManager->SetUserInitialization(phys);

    // Set user action classes
    runManager->SetUserInitialization(new DicomActionInitialization());

    runManager->Initialize();

#ifdef G4VIS_USE
    // visualisation manager
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();
#endif

    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    if (argc==1)
    {
#ifdef G4UI_USE
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);

#ifdef G4VIS_USE
        UImanager->ApplyCommand("/control/execute vis.mac");
#endif
        ui->SessionStart();
        delete ui;
#endif
    }
    else
    {
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command+fileName);
    }

    delete runManager;

#ifdef G4VIS_USE
    delete visManager;
#endif

    delete dcmHandler;

    return 0;
}
