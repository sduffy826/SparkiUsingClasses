#!/bin/bash


# copy the localizationCommon.h file to the determineWorld code (and any others)
rsync -av localizationClassTest/localizationClass.h determineWorldClassTest/
rsync -av localizationClassTest/localizationClass.cpp determineWorldClassTest/

rsync -av localizationClassTest/localizationClass.h movementsClassTest/
rsync -av localizationClassTest/localizationClass.cpp movementsClassTest/

rsync -av localizationClassTest/localizationClass.h lightsClassTest/
rsync -av localizationClassTest/localizationClass.cpp lightsClassTest/

rsync -av localizationClassTest/localizationClass.h ultrasonicClassTest/
rsync -av localizationClassTest/localizationClass.cpp ultrasonicClassTest/

rsync -av localizationClassTest/localizationClass.h infraredClassTest/
rsync -av localizationClassTest/localizationClass.cpp infraredClassTest/

# copy the movements/movements.h into paths
rsync -av movementsClassTest/movementsClass.h lightsClassTest/
rsync -av movementsClassTest/movementsClass.cpp lightsClassTest/

rsync -av movementsClassTest/movementsClass.h infraredClassTest/
rsync -av movementsClassTest/movementsClass.cpp infraredClassTest/

# copy the determineWorld.h into the appropriate paths
rsync -av determineWorldClassTest/determineWorldClass.h movementsClassTest/
rsync -av determineWorldClassTest/determineWorldClass.cpp movementsClassTest/

rsync -av determineWorldClassTest/determineWorldClass.h lightsClassTest/
rsync -av determineWorldClassTest/determineWorldClass.cpp lightsClassTest/

rsync -av determineWorldClassTest/determineWorldClass.h ultrasonicClassTest/
rsync -av determineWorldClassTest/determineWorldClass.cpp ultrasonicClassTest/

rsync -av determineWorldClassTest/determineWorldClass.h infraredClassTest/
rsync -av determineWorldClassTest/determineWorldClass.cpp infraredClassTest/

#rsync -av determineWorldClassTest/determineWorldClass.h lightsClassTest/
#rsync -av determineWorldClassTest/determineWorldClass.cpp classClassTest/

# copy the ultrasonic code to the right paths
rsync -av ultrasonicClassTest/ultrasonicClass.h determineWorldClassTest/
rsync -av ultrasonicClassTest/ultrasonicClass.cpp determineWorldClassTest/

rsync -av ultrasonicClassTest/ultrasonicClass.h movementsClassTest/
rsync -av ultrasonicClassTest/ultrasonicClass.cpp movementsClassTest/

rsync -av ultrasonicClassTest/ultrasonicClass.h lightsClassTest/
rsync -av ultrasonicClassTest/ultrasonicClass.cpp lightsClassTest/

rsync -av ultrasonicClassTest/ultrasonicClass.h infraredClassTest/
rsync -av ultrasonicClassTest/ultrasonicClass.cpp infraredClassTest/

# copy the sparki constants into the associated directories
rsync -av SparkiCommonClass/sparkiClassCommon.h determineWorldClassTest/
rsync -av SparkiCommonClass/sparkiClassCommon.h movementsClassTest/
rsync -av SparkiCommonClass/sparkiClassCommon.h localizationClassTest/
rsync -av sparkiCommonClass/sparkiClassCommon.h ultrasonicClassTest/
rsync -av SparkiCommonClass/sparkiClassCommon.h lightsClassTest/
rsync -av SparkiCommonClass/sparkiClassCommon.h infraredClassTest/
rsync -av SparkiCommonClass/StackArray.h infraredClassTest/
# rsync -av SparkiCommonClass/SplitString.h infraredClassTest/

# rsync -av SparkiCommonClass/sparkiClassCommon.cpp determineWorldClassTest/
# rsync -av SparkiCommonClass/sparkiClassCommon.cpp movementsClassTest/
# rsync -av SparkiCommonClass/sparkiClassCommon.cpp localizationClassTest/
# rsync -av sparkiCommonClass/sparkiClassCommon.cpp ultrasonicClassTest/
# rsync -av SparkiCommonClass/sparkiClassCommon.cpp lightsClassTest/