#ifndef REGENERATORASSIGNMENTALGORITHM_H
#define REGENERATORASSIGNMENTALGORITHM_H

#include <memory>
#include <Structure/Topology.h>
#include <Calls/Call.h>
#include <RWA/Route.h>

/**
 * @brief The RegeneratorAssignmentAlgorithm class assigns which nodes will use
 * regenerators, given a call and a set of links found by the RoutingAlgorithm.
 */
class RegeneratorAssignmentAlgorithm {
  public:
    /**
     * @brief RegeneratorBitrate is the maximum bitrate that a single
     * Regenerator can regenerate. It's measured in bits per second.
     */
    constexpr static long double RegeneratorBitrate = 100E9;

    /**
     * @brief RegeneratorAssignmentAlgorithm is the standard constructor for a
     * RegeneratorAssignmentAlgorithm.
     * @param T is a pointer to the Topology.
     * @param Schemes is a vector containing the possible modulation schemes.
     */
    RegeneratorAssignmentAlgorithm(std::shared_ptr<Topology> T,
                                   std::vector<ModulationScheme> Schemes);

    /**
     * @brief T is a pointer to the Topology.
     */
    std::shared_ptr<Topology> T;
    /**
     * @brief ModulationSchemes is a vector containing the possible modulation
     * schemes.
     */
    std::vector<ModulationScheme> ModulationSchemes;

    /**
     * @brief assignRegenerators decides which nodes will use their regeneration
     * capabilities.
     * @param C is a pointer to the Call.
     * @param Links is the set of Links that the Call will traverse.
     * @return a vector of TransparentSegment, each one except the last ended with
     * a translucent node.
     */
    virtual std::vector<TransparentSegment> assignRegenerators(
        std::shared_ptr<Call> C,
        std::vector<std::weak_ptr<Link>> Links) = 0;

    /**
     * @brief get_NumNeededRegenerators returns the number of regenerators needed
     * to regenerate the Call.
     * @param C is a pointer to the call.
     * @return the number of regenerators needed to regenerate the Call.
     */
    unsigned int get_NumNeededRegenerators(std::shared_ptr<Call> C);
    /**
     * @brief isThereSpectrumAndOSNR returns true if there's OSNR and Spectrum
     * to implement the Call from start to end Nodes.
     * @param C is a pointer to the Call.
     * @param Links is a vector with the Links that the call will traverse.
     * @param start is a pointer to the start Node.
     * @param end is a pointer to the end Node.
     * @return true iff there's spectrum and OSNR enough to implement the Call.
     */
    bool isThereSpectrumAndOSNR(std::shared_ptr<Call> C,
                                std::vector<std::weak_ptr<Link>> Links,
                                std::weak_ptr<Node> start,
                                std::weak_ptr<Node> end);
    /**
     * @brief isThereSpectrumAndOSNR returns true if there's OSNR and Spectrum
     * to implement the Call from start to end Nodes, with a specific scheme.
     * @param C is a pointer to the Call.
     * @param Links is a vector with the Links that the call will traverse.
     * @param start is a pointer to the start Node.
     * @param end is a pointer to the end Node.
     * @param scheme is the ModulationScheme being tested.
     * @return true iff there's spectrum and OSNR enough to implement the Call.
     */
    bool isThereSpectrumAndOSNR(std::shared_ptr<Call> C,
                                std::vector<std::weak_ptr<Link>> Links,
                                std::weak_ptr<Node> start,
                                std::weak_ptr<Node> end,
                                ModulationScheme scheme);
    ModulationScheme getMostEfficientScheme(std::shared_ptr<Call> C,
                                            std::vector<std::weak_ptr<Link>> SegmentLinks);
    /**
     * @brief createTransparentSegment breaks the set of links into a subset that
     * starts with the start Node and ends with the end Node.
     * @param C is a pointer to the Call.
     * @param Links is a vector with the Links that the call will traverse.
     * @param start is a pointer to the start Node.
     * @param end is a pointer to the end Node.
     * @param NumRegUsed is the number of regenerators used in this TransparentSegment.
     * @return the broken TransparentSegment.
     */
    TransparentSegment createTransparentSegment(std::shared_ptr<Call> C,
            std::vector<std::weak_ptr<Link>> Links,
            std::weak_ptr<Node> start,
            std::weak_ptr<Node> end,
            unsigned int NumRegUsed);
  private:
    std::vector<std::weak_ptr<Link>> segmentLinks(
                                      std::vector<std::weak_ptr<Link>>Links,
                                      std::weak_ptr<Node> start,
                                      std::weak_ptr<Node> end);
};

#endif // REGENERATORASSIGNMENTALGORITHM_H