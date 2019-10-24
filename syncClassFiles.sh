#!/bin/bash


# copy the localizationCommon.h file to the determineWorld code (and any others)
rsync -av localizationClassTest/localizationClass.h determineWorldClassTest/
rsync -av localizationClassTest/localizationClass.cpp determineWorldClassTest/

rsync -av localizationClassTest/localizationClass.h movementsClassTest/
rsync -av localizationClassTest/localizationClass.cpp movementsClassTest/

rsync -av localizationClassTest/localizationClass.h lightsClassTest/
rsync -av localizationClassTest/localizationClass.cpp lightsClassTest/

# copy the movements/movements.h into paths
rsync -av movementsClassTest/movementsClass.h lightsClassTest/
rsync -av movementsClassTest/movementsClass.cpp lightsClassTest/

# copy the determineWorld.h into the appropriate paths
rsync -av determineWorldClassTest/determineWorldClass.h movementsClassTest/
rsync -av determineWorldClassTest/determineWorldClass.cpp movementsClassTest/

rsync -av determineWorldClassTest/determineWorldClass.h lightsClassTest/
rsync -av determineWorldClassTest/determineWorldClass.cpp lightsClassTest/

#rsync -av determineWorldClassTest/determineWorldClass.h lightsClassTest/
#rsync -av determineWorldClassTest/determineWorldClass.cpp classClassTest/

# copy the ultrasonic code to the right paths
rsync -av ultrasonicClassTest/ultrasonicClass.h determineWorldClassTest/
rsync -av ultrasonicClassTest/ultrasonicClass.cpp determineWorldClassTest/

rsync -av ultrasonicClassTest/ultrasonicClass.h movementsClassTest/
rsync -av ultrasonicClassTest/ultrasonicClass.cpp movementsClassTest/

rsync -av ultrasonicClassTest/ultrasonicClass.h lightsClassTest/
rsync -av ultrasonicClassTest/ultrasonicClass.cpp lightsClassTest/

# copy the sparki constants into the associated directories
rsync -av SparkiCommonClass/sparkiClassCommon.h determineWorldClassTest/
rsync -av SparkiCommonClass/sparkiClassCommon.h movementsClassTest/
rsync -av SparkiCommonClass/sparkiClassCommon.h localizationClassTest/
rsync -av sparkiCommonClass/sparkiClassCommon.h ultrasonicClassTest/
rsync -av SparkiCommonClass/sparkiClassCommon.h lightsClassTest/