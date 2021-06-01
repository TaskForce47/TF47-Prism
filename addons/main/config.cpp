#include "script_component.hpp"

class CfgPatches {
    class tf47_prism_main {
        name = "Gruppe Adler Intercept Template";
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.92;
        requiredAddons[] = {"intercept_core"};
        authors[] = { "TF47 Dragon" };
        url = "https://github.com/TaskForce47/TF47-Prism";
        VERSION_CONFIG;
    };
};
class Intercept {
    class grad {
        class tf47_prism {
            pluginName = "tf47_prism";
        };
    };
};
