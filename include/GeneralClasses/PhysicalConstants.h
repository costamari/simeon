#ifndef PHYSICALCONSTANTS_H
#define PHYSICALCONSTANTS_H

/**
 * @brief The PhysicalConstants class stores physical constants.
 */
class PhysicalConstants
{
public:
    /**
     * @brief c is the light's velocity.
     */
    constexpr static double c = 299792458;
    /**
     * @brief h is the Planck's constant.
     */
    constexpr static double h = 6.62606957E-34;
    /**
     * @brief lambda is the light's wavelength.
     */
    constexpr static double lambda = 1550E-9;
    /**
     * @brief freq is the light's frequency;
     */
    constexpr static double freq = 193.4E12;
    /**
     * @brief BRef is the reference bandwidth.
     */
    static constexpr double BRef = 12.5E9;
    /**
     * @brief numPolarizations is used to choose whether
     * one or two polarizations are used to transmit the signal.
     */
    constexpr static int numPolarizations = 2;
};

#endif // PHYSICALCONSTANTS_H

