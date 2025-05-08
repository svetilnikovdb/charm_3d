#include "charm_globals.h"

#ifndef CHARM_3D_COMPONENT_H
#define CHARM_3D_COMPONENT_H
#define M_H 1.00797
#define M_O 15.9994

#endif

typedef enum {
    H, O, H2, O2, OH, H2O, HO2
} Component;

typedef struct { ////! добавить данные по аналогии с task.yml components
    Component component;
    charm_real_t M;
} ComponentInfo;

ComponentInfo getComponentInfo(Component component) {
    ComponentInfo info;
    switch (component) {
        case H:
            info.component = component;
            info.M = M_H;
            return info;
        case O:
            info.component = component;
            info.M = M_O;
            return info;
        case H2:
            info.component = component;
            info.M = 2*M_H;
            return info;
        case O2:
            info.component = component;
            info.M = 2*M_O;
            return info;
        case OH:
            info.component = component;
            info.M = M_O + M_H;
            return info;
        case H2O:
            info.component = component;
            info.M = 2*M_H + M_O;
            return info;
        case HO2:
            info.component = component;
            info.M = M_H + 2*M_O;
            return info;
    }
}