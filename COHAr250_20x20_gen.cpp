///////////////////////////////////////////
// COHAr250_gen.cpp
///////////////////////////////////////////
//  Author : Yun-Tse Tsai
//  Date   : June 25th, 2026
//  Version: v0.0
//  Make a simple code to generate cosmic muons from EcoMug,
//  Hardcoded everywhere in this first version.
//
//  Cosmic rays: Use the EcoMug default, 129Hz/m2
//  Generate 20x20m2, from -30µs to 202µs
//  Expect 3 cosmic rays per event, use a Poisson distribution with
//  the mean value of 3
//  Output units: GeV, cm, ns
/////////////////////////////////////////////


#include "EcoMug.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>

// Uniform real in [t1, t2)
double rand_time(double t1, double t2) {
    static thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<double> dist(t1, t2);
    return dist(rng);
}

// Poisson-distributed integer with mean (lambda)
int rand_poisson(double lambda) {
    static thread_local std::mt19937 rng{ std::random_device{}() };
    std::poisson_distribution<int> dist(lambda);
    return dist(rng);
}

// main
int main(int argc, char* argv[]) {

    int startFile = 0;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <integer>\n";
        return 1;
    }

    try {
        std::string s = argv[1];
        size_t pos = 0;
        int value = std::stoi(s, &pos);

        // Ensure entire string was a valid integer (e.g., reject "123abc")
        if (pos != s.size()) {
            std::cerr << "Invalid integer: " << s << "\n";
            return 1;
        }

        std::cout << "Parsed value = " << value << "\n";
	startFile = value;
    }
    catch (const std::invalid_argument&) {
        std::cerr << "Not a number: " << argv[1] << "\n";
        return 1;
    }
    catch (const std::out_of_range&) {
        std::cerr << "Number out of int range: " << argv[1] << "\n";
        return 1;
    }

    std::cout << "Starting generation at file " << startFile << "\n";

    EcoMug gen;
    gen.SetUseSky();
    gen.SetSkySize({{20.0, 20.0}}); // m
    gen.SetSkyCenterPosition({{0., 0., 6.81}}); // m

    double muMass = 0.105658;  // GeV
    int PID = 13;
    int nParticlePerVertex = 1;

    int ISTHEP = 1;
    int JMOHEP1 = 0;
    int JMOHEP2 = 0;
    int JDAHEP1 = 0;
    int JDAHEP2 = 0;

    int nFiles = 1000;
    int nEventsPerFile = 100000;

    for ( int iFile = startFile; iFile < startFile + nFiles; iFile++ ) {

        std::ostringstream oss;
        oss << "/sdf/data/neutrino/yuntse/coherent/SNeNDSens/gen/Cosmics_20x20/" << std::setw(5) << std::setfill('0') 
            << iFile/100 *100 << "/CosmicFlux_" << std::setw(5) << std::setfill('0') << iFile << ".hepevt";
        std::ofstream outfile(oss.str());
        
        for ( int iEvent = 0; iEvent < nEventsPerFile; iEvent++ ) {

            int avgnMuons = 12;
            int nMuons = rand_poisson( avgnMuons );

            for ( int iMuon = 0; iMuon < nMuons; iMuon++ ) {

                gen.Generate();

                // position
                std::array<double, 3> xyz = gen.GetGenerationPosition();
                // time
                double t = rand_time( -30000, 202000); // ns

                // momentum
                std::array<double, 3> pxyz;
                gen.GetGenerationMomentum(pxyz);  // px, py, pz, GeV/c
                // energy
                double E = std::sqrt(pxyz[0]*pxyz[0] + pxyz[1]*pxyz[1] + pxyz[2]*pxyz[2] + muMass*muMass);
            
                // Pdg id
                int charge = gen.GetCharge();
                if (charge > 0) PID = 13;
                else PID = -13;
            
                int EventNo = iFile*nEventsPerFile + iEvent;
                outfile << EventNo << " " << iMuon << " " << nParticlePerVertex << "\n";
                // Output units: GeV, cm, ns
                outfile << ISTHEP << " " << PID << " " << JMOHEP1 << " " << JMOHEP2 << " " << JDAHEP1 << " " << JDAHEP2 << " " 
                        << pxyz[0] << " " << pxyz[1] << " " << pxyz[2] << " " << E << " "
                        << muMass << " " << xyz[0]*100. << " " << xyz[1]*100. << " " << xyz[2]*100. << " " << t << "\n";
                
            }
        }
        // std::cout << "Estimated time [s] = " << gen.GetEstimatedTime(nEventsPerFile) << std::endl;
    }
    return 0;
}
