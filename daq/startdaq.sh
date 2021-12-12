EVENTS=10
RUN=${1}
DRS_DIRECTORY="."
WC_DIRECTORY="."
CHERENKOV_DIRECTORY="."

echo "*************************************"
echo "Call for at least ${EVENTS} events"
echo "Files will be stored in"
echo " - DRS: ${DRS_DIRECTORY}/Run_${RUN}.dat"
echo " - WC: ${WC_DIRECTORY}/Run_${RUN}.dat"
echo " - CHERENKOV: ${CHERENKOV_DIRECTORY}/Run_{RUN}.dat"
echo "*************************************"
