#include <SimulationTypes/Simulation_NSGA2_RegnPlac.h>
#include <GeneralClasses/RandomGenerator.h>
#include <SimulationTypes/NetworkSimulation.h>
#include <GeneralClasses/TransmissionBitrate.h>
#include <GeneralClasses/ModulationScheme.h>
#include <Structure/Link.h>
#include <Calls/CallGenerator.h>
#include <RMSA/RoutingWavelengthAssignment.h>
#include <RMSA/RegeneratorAssignmentAlgorithms/RegeneratorAssignmentAlgorithm.h>
#include <RMSA/RegeneratorPlacementAlgorithms/MultiObjectiveRegeneratorPlacement/NSGA2_Parameters.h>
#include <include/GeneralPurposeAlgorithms/NSGA-2.h>
#include <algorithm>
#include <boost/assign.hpp>
#include <boost/program_options.hpp>

using namespace Simulations;
using namespace RMSA::RA;
using namespace RMSA::ROUT;
using namespace RMSA::SA;
using namespace NSGA_II;

unsigned int Simulation_NSGA2_RegnPlac::RegnMax;

Simulation_NSGA2_RegnPlac::Simulation_NSGA2_RegnPlac() :
    SimulationType(Simulation_Type::morp3o), hasLoaded(false)
{
    auto maxBitrate = std::max_element(TransmissionBitrate::DefaultBitrates.begin(),
                                       TransmissionBitrate::DefaultBitrates.end());
    RegnMax = Link::NumSlots * std::ceil(maxBitrate->get_Bitrate() /
                                         RegeneratorAssignmentAlgorithm::RegeneratorBitrate);
}

void Simulation_NSGA2_RegnPlac::Individual::createIndividual()
{
    std::vector<int> IndivGene;

    for (size_t gene = 0; gene < Sim.T->Nodes.size(); gene++)
        {
        IndivGene.push_back(createGene(gene));
        }

    setGene(IndivGene);
}

int Simulation_NSGA2_RegnPlac::Individual::createGene(unsigned int)
{
    std::uniform_real_distribution<double> isTranslucent(0, 1);

    if (isTranslucent(random_generator) < 0.5)
        {
        return std::uniform_int_distribution<unsigned int>
               (0, Simulation_NSGA2_RegnPlac::RegnMax)(random_generator);
        }
    else
        {
        return 0;
        }
}

std::shared_ptr<NSGA2_Individual>
Simulation_NSGA2_RegnPlac::Individual::clone()
{
    std::shared_ptr<Individual> indiv(new Individual(Sim));
    indiv->setGene(Gene);

    indiv->isEvaluated = isEvaluated;
    if (isEvaluated)
        {
        for (unsigned i = 0; i < getNumParameters(); i++)
            {
            indiv->getParameter(i)->setValue(getParameter(i)->evaluate());
            }
        }

    return indiv;
}

void Simulation_NSGA2_RegnPlac::Individual::setGene(
    std::vector<int> gene)
{
    using namespace RMSA::RP::MORP;
    Gene = gene;
    Parameters.clear();
    Parameters.push_back(std::shared_ptr<NSGA2_Parameter>(
                             new NSGA2_Parameter_NumberOfRegenerators(Gene, Sim)));
    Parameters.push_back(std::shared_ptr<NSGA2_Parameter>(
                             new NSGA2_Parameter_BlockingProbability(Gene, Sim)));
}

Simulation_NSGA2_RegnPlac::Sim_NSGA2::Sim_NSGA2(Simulation_NSGA2_RegnPlac &Sim)
    : NSGA2(), Sim(Sim)
{

}

void Simulation_NSGA2_RegnPlac::Sim_NSGA2::createInitialGeneration()
{
    std::shared_ptr<NSGA2_Generation> gen(new NSGA2_Generation());

    for (unsigned i = 0; i < NSGA2::numIndiv; i++)
        {
        std::shared_ptr<Individual> indiv(new Individual(Sim));
        indiv->createIndividual();

        gen->operator +=(indiv);
        }

    evolution.push_back(gen);
}

void Simulation_NSGA2_RegnPlac::help()
{
    std::cout << "\t\tMORP-3O REGENERATOR PLACEMENT" << std::endl << std::endl <<
              "This simulation uses the NSGA-2 genetic algorithm to find a"
              " suitable set of transparent and translucent nodes, in order to"
              " fit simultaneously call blocking probability, CapEx and OpEx"
              " constraints." << std::endl;
}

void Simulation_NSGA2_RegnPlac::save(std::string SimConfigFileName)
{
    SimulationType::save(SimConfigFileName);
    Link::save(SimConfigFileName, T);

    std::ofstream SimConfigFile(SimConfigFileName,
                                std::ofstream::out | std::ofstream::app);

#ifdef RUN_ASSERTIONS
    if (!SimConfigFile.is_open())
        {
        std::cerr << "Output file is not open" << std::endl;
        abort();
        }
#endif

    SimConfigFile << std::endl << "  [algorithms]" << std::endl;
    SimConfigFile << "  RoutingAlgorithm = " <<
                  RoutingAlgorithm::RoutingAlgorithmNicknames.left.at(Routing_Algorithm) <<
                  std::endl;
    SimConfigFile << "  WavelengthAssignmentAlgorithm = " <<
                  SpectrumAssignmentAlgorithm::SpectrumAssignmentAlgorithmNicknames.left.at(
                      WavAssign_Algorithm) << std::endl;
    SimConfigFile << "  RegeneratorAssignmentAlgorithm = " <<
                  RegeneratorAssignmentAlgorithm::RegeneratorAssignmentNicknames.left.at(
                      RegAssignment_Algorithm) << std::endl;

    SimConfigFile << std::endl << "  [sim_info]" << std::endl << std::endl;
    SimConfigFile << "  NumCalls = " << NumCalls << std::endl;
    SimConfigFile << "  NetworkLoad = " << NetworkLoad << std::endl;

    SimConfigFile << std::endl;
    T->save(SimConfigFileName);
}

void Simulation_NSGA2_RegnPlac::load_file(std::string ConfigFileName)
{
    using namespace boost::program_options;

    options_description ConfigDesctription("Configurations Data");
    ConfigDesctription.add_options()("general.SimulationType",
                                     value<std::string>()->required(), "Simulation Type")
    ("general.AvgSpanLength", value<long double>()->required(),
     "Distance Between Inline Amps.")
    ("algorithms.RoutingAlgorithm", value<std::string>()->required(),
     "Routing Algorithm")
    ("algorithms.WavelengthAssignmentAlgorithm", value<std::string>()->required(),
     "Wavelength Assignment Algorithm")
    ("algorithms.RegeneratorAssignmentAlgorithm", value<std::string>()->required(),
     "Regenerator Assignment Algorithm")
    ("sim_info.NumCalls", value<long double>()->required(), "Number of Calls")
    ("sim_info.NetworkLoad", value<long double>()->required(), "Network Load");

    variables_map VariablesMap;

    std::ifstream ConfigFile(ConfigFileName, std::ifstream::in);
#ifdef RUN_ASSERTIONS
    if (!ConfigFile.is_open())
        {
        std::cerr << "Input file is not open" << std::endl;
        abort();
        }
#endif
    store(parse_config_file<char>(ConfigFile, ConfigDesctription, true),
          VariablesMap);
    ConfigFile.close();
    notify(VariablesMap);

    T = std::shared_ptr<Topology>(new Topology(ConfigFileName));
    Link::DefaultAvgSpanLength =
        VariablesMap["general.AvgSpanLength"].as<long double>();
    T->set_avgSpanLength(VariablesMap["general.AvgSpanLength"].as<long double>());
    Routing_Algorithm = RoutingAlgorithm::RoutingAlgorithmNicknames.right.at(
                            VariablesMap["algorithms.RoutingAlgorithm"].as<std::string>());
    WavAssign_Algorithm =
        SpectrumAssignmentAlgorithm::SpectrumAssignmentAlgorithmNicknames.right.at(
            VariablesMap["algorithms.WavelengthAssignmentAlgorithm"].as<std::string>());
    RegAssignment_Algorithm =
        RegeneratorAssignmentAlgorithm::RegeneratorAssignmentNicknames.right.at(
            VariablesMap["algorithms.RegeneratorAssignmentAlgorithm"].as<std::string>());
    NumCalls = VariablesMap["sim_info.NumCalls"].as<long double>();
    NetworkLoad = VariablesMap["sim_info.NetworkLoad"].as<long double>();

    hasLoaded = true;
}

void Simulation_NSGA2_RegnPlac::print()
{
    if (!hasLoaded)
        {
        load();
        }

    std::cout << std::endl <<
              "  A MORP-3O Regenerator Placement Simulation is about to start with the following parameters: "
              << std::endl;
    std::cout << "-> Metrics =" << std::endl;
    for(auto &metric : Metrics)
        {
        std::cout << "\t-> " << SimulationType::MetricTypes.left.at(
                      metric) << std::endl;
        }
    if(considerFilterImperfection)
        {
        std::cout << "-> Filter Order = " << SpectralDensity::GaussianOrder <<
                  std::endl;
        }
    std::cout << "-> Distance Between Inline Amps. = " << T->AvgSpanLength <<
              std::endl;
    std::cout << "-> Routing Algorithm = " <<
              RoutingAlgorithm::RoutingAlgorithmNames.left.at(Routing_Algorithm)
              << std::endl;
    std::cout << "-> Wavelength Assignment Algorithm = " <<
              SpectrumAssignmentAlgorithm::SpectrumAssignmentAlgorithmNames.left.at(
                  WavAssign_Algorithm)
              << std::endl;
    std::cout << "-> Regenerator Assignment Algorithm = " <<
              RegeneratorAssignmentAlgorithm::RegeneratorAssignmentNames.left.at(
                  RegAssignment_Algorithm) << std::endl;
    std::cout << "-> Number of Calls = " << NumCalls << std::endl;
    std::cout << "-> Network Load = " << NetworkLoad << std::endl;
}

void Simulation_NSGA2_RegnPlac::load()
{
    //Generic readings.
    SimulationType::load();

    Node::load();

    Link::load(T);

    //RMSA Algorithms
        {
        //Routing Algorithm
        Routing_Cost = RoutingCost::define_RoutingCost();
        Routing_Algorithm = RoutingAlgorithm::define_RoutingAlgorithm();

        //Wavelength Assignment Algorithm
        WavAssign_Algorithm =
            SpectrumAssignmentAlgorithm::define_SpectrumAssignmentAlgorithm();

        //Regenerator Assignment Algorithm
        RegAssignment_Algorithm =
            RegeneratorAssignmentAlgorithm::define_RegeneratorAssignmentAlgorithm();
        }

    std::cout << std::endl << "-> Define the number of calls." << std::endl;

    do
        {
        std::cin >> NumCalls;

        if (std::cin.fail() || NumCalls < 0)
            {
            std::cin.clear();
            std::cin.ignore();

            std::cerr << "Invalid number of calls." << std::endl;
            std::cout << std::endl << "-> Define the number of calls." << std::endl;
            }
        else
            {
            break;
            }
        }
    while (1);

    std::cout << std::endl << "-> Define the network load." << std::endl;

    do
        {
        std::cin >> NetworkLoad;

        if (std::cin.fail() || NetworkLoad < 0)
            {
            std::cin.clear();
            std::cin.ignore();

            std::cerr << "Invalid network load." << std::endl;
            std::cout << std::endl << "-> Define the network load." << std::endl;
            }
        else
            {
            break;
            }
        }
    while (1);

    std::cout << std::endl << "-> Define the file where to store the results."
              << std::endl;
    do
        {
        std::cin >> FileName;

        if (std::cin.fail())
            {
            std::cin.clear();
            std::cin.ignore();

            std::cerr << "Invalid filename." << std::endl;
            std::cout << std::endl << "-> Define the file where to store the results."
                      << std::endl;
            }
        else
            {
            break;
            }
        }
    while (1);

    hasLoaded = true;
}

void Simulation_NSGA2_RegnPlac::run()
{
    if (!hasLoaded)
        {
        load();
        }

    Sim_NSGA2 Optimization(*this);

    std::cout << std::endl << "* * RESULTS * *" << std::endl;
    std::cout << "FIRST PARETO FRONT OF I-TH GENERATION" << std::endl;

    while (Optimization.generation < NSGA2::numGen)
        {
        Optimization.run_Generation();
        std::cout << std::endl << "GENERATION " <<  Optimization.generation
                  << std::endl;
        Optimization.evolution.at(Optimization.generation - 1)->print();
        Optimization.evolution.at(Optimization.generation - 1)->print(
            FileName + "_G" + std::to_string(Optimization.generation), 1);
        }

    // Saving Sim. Configurations
    std::string ConfigFileName = "SimConfigFile.ini"; // Name of the file
    save(ConfigFileName);
}
