#ifndef PARTICLESWARMOPTIMIZATION_H
#define PARTICLESWARMOPTIMIZATION_H

#include <GeneralPurposeAlgorithms/PSO/PSO_Particle.h>
#include <vector>
#include <algorithm>
#include <iostream>

namespace PSO {
    template<class T, class Fit, class Comp>
    class ParticleSwarmOptimization {
      public:
        ParticleSwarmOptimization(unsigned int P, unsigned int G, unsigned int N,
                                  T XMin, T XMax, T VMin, T VMax);

        std::vector<std::shared_ptr<PSO_Particle<T>>> Particles;

        std::shared_ptr<PSO_Particle<T>> BestParticle;

        void run_generation();

      private:
        unsigned int P;
        unsigned int G;
        unsigned int N;

        int g;
        int p;

        void updatePositions();

        T XMin;
        T XMax;
        T VMin;
        T VMax;

        static constexpr double c1 = 2.05;
        static constexpr double c2 = 2.05;
        static constexpr double phi = c1 + c2;
        static constexpr double chi = 2.0 / std::abs(2 - phi - sqrt(
                                          phi * phi - 4 * phi));

    };

    template<class T, class Fit, class Comp>
    PSO::ParticleSwarmOptimization<T, Fit, Comp>::ParticleSwarmOptimization(
        unsigned int P,
        unsigned int G,
        unsigned int N,
        T XMin,
        T XMax,
        T VMin,
        T VMax) :
        P(P), G(G), N(N), XMin(XMin), XMax(XMax), VMin(VMin), VMax(VMax) {

        //Create Particles
        Particles.resize(P);

        for (auto particle = Particles.begin(); particle != Particles.end();
                ++particle) {
            *particle = std::shared_ptr<PSO_Particle<T>>(
                            new PSO_Particle<T>(N, XMin, XMax));
        }

        //Create ring topology
        for (unsigned particle = 0; particle < Particles.size(); ++particle) {
            Particles[particle]->Neighbour[0] = Particles[(particle + 1) % P];
            Particles[particle]->Neighbour[1] = Particles[(particle - 1) % P];
        }

        BestParticle = Particles[0];
        p = g = 1;
    }

    template<class T, class Fit, class Comp>
    void PSO::ParticleSwarmOptimization<T, Fit, Comp>::run_generation() {
        p = 1;

        for (auto particle : Particles) {
            particle->currentFit = Fit()(particle);

            if (Comp()(particle->currentFit, particle->bestFit) || g == 1) {
                particle->bestFit = particle->currentFit;
                particle->P = particle->X;
            }

            if (Comp()(particle->bestFit, BestParticle->bestFit)) {
                std::clog << "New fitter particle found. Fit: " << particle->currentFit
                          << std::endl;

                BestParticle = particle;
            }

            p++;
        }

        updatePositions();
        g++;
    }

    template<class T, class Fit, class Comp>
    void PSO::ParticleSwarmOptimization<T, Fit, Comp>::updatePositions() {

        std::uniform_real_distribution<double> PSO_UnifDistribution(0, 1);

        for (auto particle : Particles) {

            auto FitterNeigh = Comp()(particle->Neighbour[0].lock()->currentFit,
                                      particle->Neighbour[1].lock()->currentFit) ?
                               particle->Neighbour[0].lock() : particle->Neighbour[1].lock();

            //Calculate velocities
            for (unsigned i = 0; i < N; i++) {
                double Eps1 = PSO_UnifDistribution(random_generator);
                double Eps2 = PSO_UnifDistribution(random_generator);

                particle->V[i] += c1 * Eps1 * (particle->P[i] - particle->X[i]);
                particle->V[i] += c2 * Eps2 * (FitterNeigh->P[i] - particle->X[i]);
                particle->V[i] *= chi;

                if (particle->V[i] > VMax) {
                    particle->V[i] = VMax;
                } else if (particle->V[i] < VMin) {
                    particle->V[i] = VMin;
                }
            }

            //Calculate positions
            for (unsigned i = 0; i < N; i++) {
                particle->X[i] += particle->V[i];

                if (particle->X[i] > XMax) {
                    particle->X[i] = XMax;
                } else if (particle->X[i] < XMin) {
                    particle->X[i] = XMin;
                }
            }
        }
    }

}

#endif // PARTICLESWARMOPTIMIZATION_H