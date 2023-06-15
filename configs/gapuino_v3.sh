if [  -n "${ZSH_VERSION:-}" ]; then
	DIR="$(readlink -f -- "${(%):-%x}")"
	DIRNAME="$(dirname $DIR)"
	GAP_SDK_HOME=$(dirname $DIRNAME)
	export GAP_SDK_HOME
	#echo $(dirname "$(readlink -f ${(%):-%N})")
else
	export GAP_SDK_HOME="$(dirname $(dirname "$(readlink -f "${BASH_SOURCE[0]}")"))"
fi

source $GAP_SDK_HOME/configs/clean.sh
source $GAP_SDK_HOME/configs/openocd-gap8.sh

export TARGET_CHIP_FAMILY="GAP8"
export TARGET_CHIP="GAP8_V3"
export TARGET_NAME="gap8_revc"
export BOARD_NAME=gapuino
export PULP_CURRENT_CONFIG=$BOARD_NAME@config_file=config/gapuino_revc.json
export GVSOC_CONFIG=gapuino_revc
export PLPBRIDGE_CABLE=ftdi@digilent
export OPENOCD_CHIP_TARGET=target/gap8revb.tcl
export OPENOCD_CABLE=interface/ftdi/gapuino_ftdi.cfg

export GAPY_TARGET=gapuino_v3

export PLPTEST_DEFAULT_PROPERTIES="chip=gap8_v3 chip_family=gap8 board=gapuino_v3 duration=50 test_duration=50"

source $GAP_SDK_HOME/configs/common.sh
