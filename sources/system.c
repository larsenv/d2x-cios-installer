#include "system.h"
bool blnUserResetRequest=false;
void onResetButtonDown() {
    blnUserResetRequest=true;
}
bool getUserResetRequestStatus() {
    return blnUserResetRequest;
}
