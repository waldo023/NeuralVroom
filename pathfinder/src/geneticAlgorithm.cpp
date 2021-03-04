#include "include/pathrouter/geneticAlgorithm.hpp"

Genotype::Genotype(vector<float> params)
{
    this->params = params;
    fitness = 0;
}
void Genotype::setRandomParams(float minVal, float maxVal)
{
    float range = maxVal - minVal;
    for(int i = 0; i < params.size(); i++)
    {
        params[i] = (((double) rand() / (RAND_MAX)) *range + minVal);
    }
}
//----------------------------------------------------------------------------------------------------------------------------------
Agent::Agent(Genotype genotype, vector<unsigned> topology, VehicleState state, vector<vector<float>> costMap)
{
    vehicleState = state;
    nn = NeuralNet(topology);
    nn.GenotypeParamsToWeights(genotype.params);
    this->costMap = costMap;
}
void Agent::update(vector<vector<int>> m)
{
    vector<double> sensorInfo = vehicleStatus.getDistanceFromObstacles(m, vehicleState);
    nn.feedForward(sensorInfo);
    vector<double> results;
    nn.getResults(results);
    vehicleState = nn.processResults(&vehicleStatus, vehicleState, results);
}
//----------------------------------------------------------------------------------------------------------------------------------
GeneticAlgorithm::GeneticAlgorithm(int genotypeParamCount, int populationSize, const vector<unsigned> &topology, mapGenerator &m)
{
    this->populationSize = populationSize;
    this->topology = topology;
    this->mapGenPtr = &m;
    currentPopulation.resize(populationSize, Genotype(vector<float>(genotypeParamCount, 0)));
    GenerationCount = 1;
    sortPopulation = true;
}
void GeneticAlgorithm::start(const int trainAmount)
{
    this->trainAmount = trainAmount;
    initPopulation();
    evaluation();
}

void GeneticAlgorithm::initPopulation()
{
    for(Genotype &geo : currentPopulation)
    {
        geo.setRandomParams(defaultInitParamMin, defaultInitParamMax);
    }
}

void GeneticAlgorithm::evaluation()
{
    agents.clear();
    vector<vector<float>> start = mapGenPtr->getStartPoints();
    vector<vector<float>> end = mapGenPtr->getEndPoints();
    Grid *grid = mapGenPtr->getGrid();
    
    for(int i = 0; i < currentPopulation.size(); i++)
    {
        VehicleState state = VehicleState{start[i][0], start[i][1], deg2rad(start[i][2]), Forward, Straight};
        vector<vector<float>> costMap = grid->nonHolonomicRelaxedCostMap(VehicleState{end[i][0], end[i][1], deg2rad(end[i][2]), Forward, Straight});
        agents.push_back(Agent(currentPopulation[i], topology, state, costMap));
    }
    int vehiclesCrashed = 0;
    //while(vehiclesCrashed < currentPopulation.size())
    //{
        vector<vector<int>> map = grid->returnRawMap();
        for(int r = 0; r < map.size(); r++)
        {
            for(int c = 0; c < map[r].size(); c++)
            {
                bool found = false;
                for(int a = 0; a < agents.size(); a++)
                {
                    if(r == (int)agents[a].vehicleState.posX && c == (int)agents[a].vehicleState.posY)
                    {
                        found = true;
                        cout << a+ 10 << " ";
                        break;
                    }
                }
                if(!found)
                {
                    cout << map[r][c] << " ";
                }
                
            }
            cout << endl;
        }
        for(int i = 0; i < agents.size(); i++)
        {
            if(agents[i].hasCrashed)
            {
                cout << "AGENT " << i << ": CRASHED" << endl;
                continue;
            }
            else if(!grid->isSafe(agents[i].vehicleState, .5))
            {
                agents[i].hasCrashed = true;
                vehiclesCrashed++;
                cout << "AGENT " << i << ": CRASHED" << endl;
                continue;
            }
            else if(agents[i].vehicleStatus.areEquivalentStates(agents[i].vehicleState, VehicleState{end[i][0], end[i][1], deg2rad(end[i][2]), Forward, Straight}))
            {
                cout << "AGENT " << i << ": SUCCEDED" << endl;
                continue;
            }
            
            agents[i].update(mapGenPtr->getMap());
            float startDist = agents[i].costMap[start[i][0]][start[i][1]];
            float currentDist = agents[i].costMap[agents[i].vehicleState.posX][agents[i].vehicleState.posY];
            currentPopulation[i].eval = (startDist - currentDist) / startDist;
            string s = "S";
            if(agents[i].vehicleState.steer == Left)
            {
                s = "L";
            }
            else if(agents[i].vehicleState.steer == Right)
            {
                s = "R";
            }
            string g = "F";
            if(agents[i].vehicleState.gear == Backward)
            {
                g = "B";
            }
            cout << "AGENT " << i << ": " << agents[i].vehicleState.posX << ", " << agents[i].vehicleState.posY << ", " 
            << agents[i].vehicleState.ori*180/M_PI << ", " << s << ", " <<  g << ", " << currentPopulation[i].eval << endl;
        //}
        cout << endl;
    }
    //evaluationFinished();
}
void GeneticAlgorithm::evaluationFinished()
{
    fitnessCalculation();
    if(sortPopulation)
    {
        std::sort(currentPopulation.begin(), currentPopulation.end());
    }
    if(GenerationCount < trainAmount)
    {
        vector<Genotype> intermediatePop = selection();
        vector<Genotype> newPopulation = recombination(intermediatePop, populationSize);
        mutation(newPopulation);
        currentPopulation = newPopulation;
        GenerationCount++;
        evaluation();
    }
}
void GeneticAlgorithm::fitnessCalculation()
{
    int populationSize = 0;
    float overallEval = 0;
    for(Genotype &geno : currentPopulation)
    {
        overallEval += geno.eval;
        populationSize++;
    }
    float averageEval = overallEval / populationSize;
    for(Genotype &geno : currentPopulation)
    {
        geno.fitness = geno.eval / overallEval;
    }
}
vector<Genotype> GeneticAlgorithm::selection()
{
    vector<Genotype> intermediatePopulation;
    intermediatePopulation.push_back(currentPopulation[0]);
    intermediatePopulation.push_back(currentPopulation[1]);
    intermediatePopulation.push_back(currentPopulation[2]);

    return intermediatePopulation;
}
vector<Genotype> GeneticAlgorithm::recombination(vector<Genotype> intermediatePop, int newPopSize)
{
    if(intermediatePop.size() < 2)
    {
        throw new invalid_argument("Intermediate population too small");
    }
    vector<Genotype> newPopulation;
    newPopulation.push_back(intermediatePop[0]);
    newPopulation.push_back(intermediatePop[1]);
    while(newPopulation.size() < newPopSize)
    {
        int rand1 = rand() % newPopSize;
        int rand2 = 0;
        do
        {
            rand2 = rand() % newPopSize;
        } while (rand1 == rand2);
        
        vector<Genotype> offspringOneAndTwo = completeCrossover(intermediatePop[rand1], intermediatePop[rand2], defaultCrossSwapProb);
        newPopulation.push_back(offspringOneAndTwo[0]);
        if(newPopulation.size() < newPopSize)
        {
            newPopulation.push_back(offspringOneAndTwo[1]);
        }
    }
    return newPopulation;
}
void GeneticAlgorithm::mutation(vector<Genotype> &newPop)
{
    for(int i = 2; i < newPop.size(); i++)
    {
        if(((double) rand() / (RAND_MAX)) < defaultMutationPerc)
        {
            mutateGenotype(newPop[i], defaultMutationProb, defaultMutationAmount);
        }
    }
}
vector<Genotype> GeneticAlgorithm::completeCrossover(const Genotype &parent1, const Genotype &parent2, float swapChance)
{
    int paramCount = parent1.params.size();
    vector<float> off1Params(paramCount);
    vector<float> off2Params(paramCount);
    for(int i = 0; i < paramCount; i++)
    {
        if(((double) rand() / (RAND_MAX)) < swapChance)
        {
            off1Params[i] = parent2.params[i];
            off2Params[i] = parent1.params[i];
        }
        else
        {
            off1Params[i] = parent1.params[i];
            off2Params[i] = parent2.params[i];
        }
    }
    return vector<Genotype>{Genotype(off1Params), Genotype(off2Params)};
}
void GeneticAlgorithm::mutateGenotype(Genotype &genotype, float mutationProb, float mutationAmount)
{
    for(float param : genotype.params)
    {
        if(((double) rand() / (RAND_MAX)) < mutationProb*100)
        {
            param += float(((double) rand() / (RAND_MAX)) * mutationAmount* 2 - mutationAmount);
        }
    }
}
vector<Genotype> GeneticAlgorithm::getPopulation()
{
    return currentPopulation;
}
void GeneticAlgorithm::printPopulation()
{
    for(Genotype &geo : currentPopulation)
    {
        for(float param : geo.params)
        {
            cout << param << " ";
        }
        cout << endl;
    }
}