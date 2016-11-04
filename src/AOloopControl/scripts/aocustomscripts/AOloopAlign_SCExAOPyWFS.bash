#!/bin/bash

execname="Cfits"


tempfile=`tempfile 2>/dev/null` || tempfile=/tmp/test$$
trap "rm -f $tempfile" 0 1 2 5 15


LINES=$( tput lines )
COLUMNS=$( tput cols )
let " nbwlines = $LINES - 10 "
let " nbwcols = $COLUMNS - 10 "
echo "$COLUMNS -> $nbwcols"





LOOPNUMBER_file="LOOPNUMBER"
confnbfile="./conf/conf_CONFNUMBER.txt"


mkdir -p conf
mkdir -p status
mkdir -p tmp


LOOPNUMBER_default=2  # loop number

# LOOPNUMBER (loop number)
if [ ! -f $LOOPNUMBER_file ]
then
	echo "creating loop number"
	echo "$LOOPNUMBER_default" > $LOOPNUMBER_file
else
	LOOPNUMBER=$(cat $LOOPNUMBER_file)
	echo "LOOPNUMBER = $LOOPNUMBER"
fi




# ======================= LOGGING =================================
LOOPNAME=$( cat LOOPNAME )
echo "LOOPNAME = $LOOPNAME"
# internal log - logs EVERYTHING
function aoconflog {
echo "$@" >> aolconf.log
dolog "$LOOPNAME" "$@"
}

# external log, less verbose
function aoconflogext {
echo "$@" >> aolconf.log
dolog "$LOOPNAME" "$@"
dologext "$LOOPNAME $@"
}





function stringcenter {
line=$1
    let " col1 = $nbwcols-35"
    columns="$col1"
    string=$(printf "%*s%*s\n" $(( (${#line} + columns) / 2)) "$line" $(( (columns - ${#line}) / 2)) " ")
}







# Filter wheels
echo "declaring filter wheels"
pyfwlist=("0" "          ")
pyfwlist+=( "1" "  OPEN    " )
pyfwlist+=( "2" "700nm 50nm" )
pyfwlist+=( "3" "  BLOCK   " )
pyfwlist+=( "4" "750nm 50nm" )
pyfwlist+=( "5" "850nm 25nm" )
pyfwlist+=( "6" "850nm 40nm" )








# Set FW <wheelNB>
function SetFW {
file="./status/status_fw.txt"
currentfw=$(echo "$(cat $file)")
if [ ! "${currentfw}" == "$1" ]
then
echo "MOVING TO FW $1"# &> ${outmesg}
pywfs wheel $1 # &> ${outmesg}
else
echo "WHEEL ALREADY IN POSITION"# > ${outmesg}
fi
sleep 0.1
currentfw=$1
echo "${currentfw}" > $file
sleep 0.1
}










state="menualign"


while true; do

stateok=0

mkdir -p status

statfile="./status/status_alignTT.txt"
TTloopstat=$(cat $statfile)
if [[ -f "$statfile" && ( "$TTloopstat" = " ON" || "$TTloopstat" = "OFF" || "$TTloopstat" = "PAU" ) ]]; then
echo "OK"
else
echo "OFF" > $statfile
TTloopstat="OFF"
fi

statfile="./status/status_alignPcam.txt"
Pcamloopstat=$(cat $statfile)
if [[ -f "$statfile" && ( "$Pcamloopstat" = " ON" || "$Pcamloopstat" = "OFF" || "$Pcamloopstat" = "PAU" ) ]]; then
echo "OK"
else
echo "OFF" > $statfile
Pcamloopstat="OFF"
fi


PyrFilter=$(cat ./status/status_fw.txt)


if [ $state = "menualign" ]; then
stateok=1
menuname="ALIGNMENT - LOOP ${LOOPNAME} ($LOOPNUMBER})\n
\n
   TT   loop is : $TTloopstat\n
   Pcam loop is : $Pcamloopstat\n
   Pyr Filter   : $PyrFilter\n"


pyTTloopgain=$(cat ./status/gain_PyAlignTT.txt)
Pcamloopgain=$(cat ./status/gain_PyAlignCam.txt)



if [ -f ./status/pcampos.txt ]; then
pywfsreimagexposref=$(cat ./status/pcampos.txt | awk '{print $1}')
pywfsreimageyposref=$(cat ./status/pcampos.txt | awk '{print $2}')
fi


stringcenter "Pyramid modulation"
menuitems=( "1 ->" "\Zb\Zr$string\Zn" )


menuitems+=( "" "" )
file="./conf/conf_pywfs_freq.txt"
if [ -f $file ]; then
pyfreq=$(cat $file)
else
pyfreq="2000"
echo "$pyfreq" > $file
fi

if [ "$pyfreq" = "500" ]; then
menuitems+=( "pyfr05" "\Zr\Z2 freq = 0.5 kHz\Zn" )
else
menuitems+=( "pyfr05" " freq = 0.5 kHz" )
fi

if [ "$pyfreq" = "1000" ]; then
menuitems+=( "pyfr10" "\Zr\Z2 freq = 1.0 kHz\Zn" )
else
menuitems+=( "pyfr10" " freq = 1.0 kHz" )
fi

if [ "$pyfreq" = "1500" ]; then
menuitems+=( "pyfr15" "\Zr\Z2 freq = 1.5 kHz\Zn" )
else
menuitems+=( "pyfr15" " freq = 1.5 kHz" )
fi

if [ "$pyfreq" = "2000" ]; then
menuitems+=( "pyfr20" "\Zr\Z2 freq = 2.0 kHz\Zn" )
else
menuitems+=( "pyfr20" " freq = 2.0 kHz" )
fi

if [ "$pyfreq" = "2500" ]; then
menuitems+=( "pyfr25" "\Zr\Z2 freq = 2.5 kHz\Zn" )
else
menuitems+=( "pyfr25" " freq = 2.5 kHz" )
fi

if [ "$pyfreq" = "3000" ]; then
menuitems+=( "pyfr30" "\Zr\Z2 freq = 3.0 kHz\Zn" )
else
menuitems+=( "pyfr30" " freq = 3.0 kHz" )
fi

if [ "$pyfreq" = "3500" ]; then
menuitems+=( "pyfr35" "\Zr\Z2 freq = 3.5 kHz\Zn" )
else
menuitems+=( "pyfr35" " freq = 3.5 kHz" )
fi


menuitems+=( "" "" )

file="./conf/conf_pywfs_modampl.txt"
if [ -f $file ]; then
pymodampl=$(cat $file)
else
pymodampl="05"
echo "$pymodampl" > $file
fi

if [ "$pymodampl" = "0.1" ]; then
menuitems+=( "pymoda01" "\Zr\Z2 modulation amplitude = 0.1\Zn" )
else
menuitems+=( "pymoda01" " modulation amplitude = 0.1" )
fi

if [ "$pymodampl" = "0.2" ]; then
menuitems+=( "pymoda02" "\Zr\Z2 modulation amplitude = 0.2\Zn" )
else
menuitems+=( "pymoda02" " modulation amplitude = 0.2" )
fi

if [ "$pymodampl" = "0.3" ]; then
menuitems+=( "pymoda03" "\Zr\Z2 modulation amplitude = 0.3\Zn" )
else
menuitems+=( "pymoda03" " modulation amplitude = 0.3" )
fi

if [ "$pymodampl" = "0.5" ]; then
menuitems+=( "pymoda05" "\Zr\Z2 modulation amplitude = 0.5\Zn" )
else
menuitems+=( "pymoda05" " modulation amplitude = 0.5" )
fi

if [ "$pymodampl" = "0.7" ]; then
menuitems+=( "pymoda07" "\Zr\Z2 modulation amplitude = 0.7\Zn" )
else
menuitems+=( "pymoda07" " modulation amplitude = 0.7" )
fi

if [ "$pymodampl" = "1.0" ]; then
menuitems+=( "pymoda10" "\Zr\Z2 modulation amplitude = 1.0\Zn" )
else
menuitems+=( "pymoda10" " modulation amplitude = 1.0" )
fi


menuitems+=( "" "" )

file="./conf/conf_pywfs_filter.txt"
if [ -f $file ]; then
pyfilter=$(cat $file)
else
pyfilter="1"
echo "$pyfilter" > $file
fi


if [ "$pyfilter" = "1" ]; then
menuitems+=( "pyfilt1" "\Zr\Z2 PyWFS filter 1  (Open)\Zn" )
else
menuitems+=( "pyfilt1" " PyWFS filter 1  (Open)" )
fi


if [ "$pyfilter" = "2" ]; then
menuitems+=( "pyfilt2" "\Zr\Z2 PyWFS filter 2  (700 nm, 50 nm BW)\Zn" )
else
menuitems+=( "pyfilt2" " PyWFS filter 2  (700 nm, 50 nm BW)" )
fi


if [ "$pyfilter" = "3" ]; then
menuitems+=( "pyfilt3" "\Zr\Z2 PyWFS filter 3  (BLOCK)\Zn" )
else
menuitems+=( "pyfilt3" " PyWFS filter 3  (BLOCK)" )
fi


if [ "$pyfilter" = "4" ]; then
menuitems+=( "pyfilt4" "\Zr\Z2 PyWFS filter 4  (750 nm, 50 nm BW)\Zn" )
else
menuitems+=( "pyfilt4" " PyWFS filter 4  (750 nm, 50 nm BW)" )
fi


if [ "$pyfilter" = "5" ]; then
menuitems+=( "pyfilt5" "\Zr\Z2 PyWFS filter 5  (850 nm, 25 nm BW)\Zn" )
else
menuitems+=( "pyfilt5" " PyWFS filter 5  (850 nm, 25 nm BW)" )
fi


if [ "$pyfilter" = "6" ]; then
menuitems+=( "pyfilt6" "\Zr\Z2 PyWFS filter 6  (850 nm, 40 nm BW)\Zn" )
else
menuitems+=( "pyfilt6" " PyWFS filter 6  (850 nm, 40 nm BW)" )
fi




menuitems+=( "" "" )

file="./conf/conf_pywfs_pickoff.txt"
if [ -f $file ]; then
pypickoff=$(cat $file)
else
pypickoff="01"
echo "$pypickoff" > $file
fi


if [ "$pypickoff" = "01" ]; then
menuitems+=( "pypick01" "\Zr\Z2 PyWFS pickoff 01  (Open)\Zn" )
else
menuitems+=( "pypick01" " PyWFS pickoff 01  (Open)" )
fi

if [ "$pypickoff" = "02" ]; then
menuitems+=( "pypick02" "\Zr\Z2 PyWFS pickoff 02  (Silver mirror)\Zn" )
else
menuitems+=( "pypick02" " PyWFS pickoff 02  (Silver mirror)" )
fi

if [ "$pypickoff" = "03" ]; then
menuitems+=( "pypick03" "\Zr\Z2 PyWFS pickoff 03  (50/50 splitter)\Zn" )
else
menuitems+=( "pypick03" " PyWFS pickoff 03  (50/50 splitter)" )
fi

if [ "$pypickoff" = "04" ]; then
menuitems+=( "pypick04" "\Zr\Z2 PyWFS pickoff 04  (650 nm SP)\Zn" )
else
menuitems+=( "pypick04" " PyWFS pickoff 04  (650 nm SP)" )
fi

if [ "$pypickoff" = "05" ]; then
menuitems+=( "pypick05" "\Zr\Z2 PyWFS pickoff 05  (700 nm SP)\Zn" )
else
menuitems+=( "pypick05" " PyWFS pickoff 05  (700 nm SP)" )
fi

if [ "$pypickoff" = "06" ]; then
menuitems+=( "pypick06" "\Zr\Z2 PyWFS pickoff 06  (750 nm SP)\Zn" )
else
menuitems+=( "pypick06" " PyWFS pickoff 06  (750 nm SP)" )
fi

if [ "$pypickoff" = "07" ]; then
menuitems+=( "pypick07" "\Zr\Z2 PyWFS pickoff 07  (800 nm SP)\Zn" )
else
menuitems+=( "pypick07" " PyWFS pickoff 07  (800 nm SP)" )
fi

if [ "$pypickoff" = "08" ]; then
menuitems+=( "pypick08" "\Zr\Z2 PyWFS pickoff 08  (850 nm SP)\Zn" )
else
menuitems+=( "pypick08" " PyWFS pickoff 08  (850 nm SP)" )
fi

if [ "$pypickoff" = "09" ]; then
menuitems+=( "pypick09" "\Zr\Z2 PyWFS pickoff 09  (750 nm LP)\Zn" )
else
menuitems+=( "pypick09" " PyWFS pickoff 09  (750 nm LP)" )
fi

if [ "$pypickoff" = "10" ]; then
menuitems+=( "pypick10" "\Zr\Z2 PyWFS pickoff 10  (800 nm LP)\Zn" )
else
menuitems+=( "pypick10" " PyWFS pickoff 10  (800 nm LP)" )
fi

if [ "$pypickoff" = "11" ]; then
menuitems+=( "pypick11" "\Zr\Z2 PyWFS pickoff 11  (850 nm LP)\Zn" )
else
menuitems+=( "pypick11" " PyWFS pickoff 11  (850 nm LP)" )
fi

if [ "$pypickoff" = "12" ]; then
menuitems+=( "pypick12" "\Zr\Z2 PyWFS pickoff 12  (Open)\Zn" )
else
menuitems+=( "pypick12" " PyWFS pickoff 12  (Open)" )
fi


pymodampl10=$(sed 's/\.//' ./conf/conf_pywfs_modampl.txt)
loopconfname="fr${pyfreq}_mod${pymodampl10}_pf${pyfilter}_pp${pypickoff}_"
echo "${loopconfname}" > ./conf/conf_loopconfname.txt


menuitems+=( "" "" )





stringcenter "Pyramid TT align"
menuitems+=( "2 ->" "\Zb\Zr$string\Zn" )

menuitems+=( "tz" "Zero TT align" )

if [ "$TTloopstat" = "OFF" ]; then
menuitems+=( "ts" "Start TT align" )
menuitems+=( "" "" )
fi

if [ "$TTloopstat" = " ON" ]; then
menuitems+=( "tp" "PAUSE TT align" )
menuitems+=( "tk" "STOP TT align (note: need to resume first if paused)" )
fi

if [ "$TTloopstat" = "PAU" ]; then
menuitems+=( "tr" "Resume TT align (after pause)" )
menuitems+=( "" "" )
fi

menuitems+=( "tg" "py TT loop gain = ${pyTTloopgain}")
menuitems+=( "tm" "Monitor TT align tmux session")
menuitems+=( "" "" )

stringcenter "Pyramid Camera Align"
menuitems+=( "3 ->" "\Zb\Zr$string\Zn" )

menuitems+=( "pz" "Zero Pcam align  ( $pywfsreimagexposref $pywfsreimageyposref )" )
menuitems+=( "pst0" "alignment step = 50000" )
menuitems+=( "pst1" "alignment step = 10000" )
menuitems+=( "pst2" "alignment step = 3000" )
menuitems+=( "pst3" "alignment step = 1000" )
menuitems+=( "pxm" "Pcam x -$pcamstep (right)" )
menuitems+=( "pxp" "Pcam x +$pcamstep (left)" )
menuitems+=( "pym" "Pcam y -$pcamstep (top)" )
menuitems+=( "pyp" "Pcam y +$pcamstep (bottom)" )

if [ "$Pcamloopstat" = "OFF" ]; then
menuitems+=( "ps" "Start Pcam align" )
menuitems+=( "" "" )
fi

if [ "$Pcamloopstat" = " ON" ]; then
menuitems+=( "pp" "PAUSE Pcam align" )
menuitems+=( "pk" "STOP Pcam align (note: need to resume first if paused)" )
fi

if [ "$Pcamloopstat" = "PAU" ]; then
menuitems+=( "pr" "RESUME Pcam align (after pause)" )
menuitems+=( "" "" )
fi

menuitems+=( "pg" "Pcam loop gain = ${Pcamloopgain}" )
menuitems+=( "pm" "Monitor Pcam align tmux session")
menuitems+=( "" "" )

stringcenter "DM flatten"
menuitems+=( "4 ->" "\Zb\Zr$string\Zn" )

menuitems+=( "fl" "Flatten DM for pyWFS" )
menuitems+=( "flk" "End flatten DM process" )
menuitems+=( "flz" "Remove flatten DM solution" )
menuitems+=( "fla" "Apply flatten DM solution" )
menuitems+=( "flm" "Monitor DM flatten tmux session" )
menuitems+=( "" "" )

stringcenter "FILTERS"
menuitems+=( "5 ->" "\Zb\Zr$string\Zn" )

menuitems+=( "fw" "Set PY Filter Wheel" )
menuitems+=( "" "" )




dialog --colors --title "Alignment" \
--ok-label "Select" \
--cancel-label "Top" \
--help-button --help-label "Exit" \
--default-item "${menualign_default}" \
 --menu "$menuname" \
  $nbwlines $nbwcols 100 "${menuitems[@]}"  2> $tempfile


retval=$?
choiceval=$(cat $tempfile)


menualign_default="$choiceval"
state="menualign"

case $retval in
   0) # button
menualign_default="$choiceval"
	case $choiceval in


	pyfr05)
pyfreq="500"
echo "${pyfreq}" > ./conf/conf_pywfs_freq.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pyfr10)
pyfreq="1000"
echo "${pyfreq}" > ./conf/conf_pywfs_freq.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pyfr15)
pyfreq="1500"
echo "${pyfreq}" > ./conf/conf_pywfs_freq.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pyfr20)
pyfreq="2000"
echo "${pyfreq}" > ./conf/conf_pywfs_freq.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pyfr25)
pyfreq="2500"
echo "${pyfreq}" > ./conf/conf_pywfs_freq.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;
	
	pyfr30)
pyfreq="3000"
echo "${pyfreq}" > ./conf/conf_pywfs_freq.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pyfr35)
pyfreq="3500"
echo "${pyfreq}" > ./conf/conf_pywfs_freq.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;



	pymoda01)
pymodampl="0.1"
echo "$pymodampl" > ./conf/conf_pywfs_modampl.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pymoda02)
pymodampl="0.2"
echo "0.2" > ./conf/conf_pywfs_modampl.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pymoda03)
pymodampl="0.3"
echo "0.3" > ./conf/conf_pywfs_modampl.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pymoda05)
pymodampl="0.5"
echo "0.5" > ./conf/conf_pywfs_modampl.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pymoda07)
pymodampl="0.7"
echo "0.7" > ./conf/conf_pywfs_modampl.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;

	pymoda10)
pymodampl="1.0"
echo "1.0" > ./conf/conf_pywfs_modampl.txt
pywfs_mod_setup ${pyfreq} ${pymodampl}
;;




	pyfilt1)
pyfilter="1"
echo "$pyfilter" > ./conf/conf_pywfs_filter.txt
pywfs_filter ${pyfilter}
;;

	pyfilt2)
pyfilter="2"
echo "$pyfilter" > ./conf/conf_pywfs_filter.txt
pywfs_filter ${pyfilter}
;;

	pyfilt3)
pyfilter="3"
echo "$pyfilter" > ./conf/conf_pywfs_filter.txt
pywfs_filter ${pyfilter}
;;

	pyfilt4)
pyfilter="4"
echo "$pyfilter" > ./conf/conf_pywfs_filter.txt
pywfs_filter ${pyfilter}
;;

	pyfilt5)
pyfilter="5"
echo "$pyfilter" > ./conf/conf_pywfs_filter.txt
pywfs_filter ${pyfilter}
;;

	pyfilt6)
pyfilter="6"
echo "$pyfilter" > ./conf/conf_pywfs_filter.txt
pywfs_filter ${pyfilter}
;;




	pypick01)
pypickoff="01"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick02)
pypickoff="02"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick03)
pypickoff="03"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick04)
pypickoff="04"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick05)
pypickoff="05"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick06)
pypickoff="06"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick07)
pypickoff="07"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick08)
pypickoff="08"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick09)
pypickoff="09"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick10)
pypickoff="10"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick11)
pypickoff="11"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;

	pypick12)
pypickoff="12"
echo "${pypickoff}" > ./conf/conf_pywfs_pickoff.txt
pywfs_pickoff ${pypickoff}
;;




	tz)
aoconflogext "TT align zero"
analog_output.py voltage C -5.0
analog_output.py voltage D -5.0
menualign_default="tz"
state="menualign"
;;
	ts)
aoconflogext "TT align loop start"
rm stop_PyAlignTT.txt
rm pause_PyAlignTT.txt
tmux kill-session -t alignPyrTT
tmux new-session -d -s alignPyrTT
tmux send-keys -t alignPyrTT "$execname -n alignPyrTT" C-m
tmux send-keys -t alignPyrTT "readshmim aol${LOOPNUMBER}_wfsdark" C-m
tmux send-keys -t alignPyrTT "cp aol${LOOPNUMBER}_wfsdark wfsdark" C-m
tmux send-keys -t alignPyrTT "readshmim aol${LOOPNUMBER}_wfsim" C-m
tmux send-keys -t alignPyrTT "scexaopywfsttalign aol${LOOPNUMBER}_wfsim" C-m
echo " ON" > ./status/status_alignTT.txt
menualign_default="tk"
state="menualign"
;; 
  	 tr)
aoconflogext "TT align loop resume" 
rm pause_PyAlignTT.txt stop_PyAlignTT.txt
if [ "$(cat ./status/status_alignTT.txt)" == "off" ]
then
dialog --title "Message" --msgbox "Starting TT align\n (CTRL-C now to abort)\n" 8 30
fi
echo " ON" > ./status/status_alignTT.txt
menualign_default="tp"
state="menualign"
;; 
	tg)
dialog --title "PyTT loop gain" --inputbox "Enter loop gain" 8 40 ${pyTTloopgain} 2> $tempfile
pyTTloopgain=$(cat $tempfile)
echo ${pyTTloopgain} > ./status/gain_PyAlignTT.txt
aoconflogext "TT align set gain ${pyTTloopgain}"
menualign_default="tg"
state="menualign"
;;
   	 tp)
touch pause_PyAlignTT.txt
aoconflogext "TT align loop pause"
echo "PAU" > ./status/status_alignTT.txt
menualign_default="tr"
state="menualign"
;;  
   	 tk) 
touch stop_PyAlignTT.txt
tmux kill-session -t alignPyrTT
echo "OFF" > ./status/status_alignTT.txt
aoconflogext "TT align loop off"
menualign_default="ts"
state="menualign"
;;
	tm) tmux a -t alignPyrTT ;; 
	pz)
aoconflogext "Pupil align zero"
pywfs_pup x home
pywfs_pup x goto $pywfsreimagexposref #150000
pywfs_pup y home
pywfs_pup y goto $pywfsreimageyposref #67000
echo "$pywfsreimagexposref $pywfsreimageyposref" > ./status/pcampos.txt
menualign_default="pz"
state="menualign"
;;
        pst0)
pcamstep=50000
menualign_default="pst0"
state="menualign"
;;
        pst1)
pcamstep=10000
menualign_default="pst1"
state="menualign"
;;
        pst2)
pcamstep=3000
menualign_default="pst2"
state="menualign"
;;
        pst3)
pcamstep=1000
menualign_default="pst3"
state="menualign"
;;
        pxm)
pywfsreimagexposref=$(($pywfsreimagexposref-$pcamstep))
pywfs_pup x goto $pywfsreimagexposref #150000
aoconflog "Pupil move x ${pywfsreimagexposref}"
echo "$pywfsreimagexposref $pywfsreimageyposref" > ./status/pcampos.txt
menualign_default="pxm"
state="menualign"
;;
        pxp)
pywfsreimagexposref=$(($pywfsreimagexposref+$pcamstep))
pywfs_pup x goto $pywfsreimagexposref #150000
aoconflog "Pupil move x ${pywfsreimagexposref}"
echo "$pywfsreimagexposref $pywfsreimageyposref" > ./status/pcampos.txt
menualign_default="pxp"
state="menualign"
;;
        pym)
pywfsreimageyposref=$(($pywfsreimageyposref-$pcamstep))
pywfs_pup y goto $pywfsreimageyposref #150000
aoconflog "Pupil move y ${pywfsreimagexposref}"
echo "$pywfsreimagexposref $pywfsreimageyposref" > ./status/pcampos.txt
menualign_default="pym"
state="menualign"
;;
        pyp)
pywfsreimageyposref=$(($pywfsreimageyposref+$pcamstep))
pywfs_pup y goto $pywfsreimageyposref #150000
aoconflog "Pupil move y ${pywfsreimagexposref}"
echo "$pywfsreimagexposref $pywfsreimageyposref" > ./status/pcampos.txt
menualign_default="pyp"
state="menualign"
;;
	ps)
aoconflogext "Pupil align loop start"
rm stop_PyAlignCam.txt
rm pause_PyAlignCam.txt
tmux kill-session -t alignPcam
tmux new-session -d -s alignPcam
tmux send-keys -t alignPcam "./aocustomscripts/alignPcam_${LOOPNAME}" C-m
echo " ON" > ./status/status_alignPcam.txt
menualign_default="pk"
state="menualign"
;; 
   	 pr)
aoconflogext "Pupil align loop resume"
rm pause_PyAlignCam.txt stop_PyAlignCam.txt
if [ "$(cat ./status/status_alignPcam.txt)" == "off" ]
then
dialog --title "Message" --msgbox "Starting Pcam align\n (CTRL-C now to abort)\n" 8 30
fi
echo " ON" > ./status/status_alignPcam.txt
menualign_default="pp"
state="menualign"
;;  
 	pg)
dialog --title "Pcam loop gain" --inputbox "Enter loop gain" 8 40 ${Pcamloopgain} 2> $tempfile
Pcamloopgain=$(cat $tempfile)
echo ${Pcamloopgain} > ./status/gain_PyAlignCam.txt
aoconflog "Pupil align loop set gain ${Pcamloopgain}"
menualign_default="pg"
state="menualign"
;;  	 
	pp) 
touch pause_PyAlignCam.txt
echo "PAU" > ./status/status_alignPcam.txt
aoconflogext "Pupil align loop pause"
menualign_default="pr"
state="menualign"
;;   
   	 pk) 
touch stop_PyAlignCam.txt
echo "OFF" > ./status/status_alignPcam.txt
tmux kill-session -t alignPcam
aoconflogext "Pupil align loop kill"
menualign_default="ps"
state="menualign"
;;   
	pm) tmux a -t alignPcam ;;
	d)
aoconflogext "Measure DM illumination"
tmux send-keys -t aolconf$LOOPNUMBER "./MeasureActMap" C-m 
menualign_default="d"
state="menualign"
;;
	fl)
aoconflogext "Flatten DM for pyWFS"
tmux kill-session -t pyrflatten
tmux new-session -d -s pyrflatten
menualign_default="fl"
state="menualign"
;;
	flk)
aoconflogext " STOP flatten DM process"
tmux kill-session -t pyrflatten
dmdispzero 5
shmim2fits dmdisp6 dmpyoffset.fits
dmdispzero 6
dm_update_channel 5 dmpyoffset.fits
menualign_default="fl"
state="menualign"
;;
	flz) dmdispzero 5 ;;
	fla) dm_update_channel 5 dmpyoffset.fits ;;
	flm) tmux a -t pyrflatten ;;
	fw)
# set FW
menualign_default="fw"
state="menupyfw"
;;

	esac;;
   1) state="menutop";;   
   2) state="menuexit";;
   255) state="menuexit";;
esac
fi










#  CONFIGURATION SETUP INDEX - FW
if [ $state = "menupyfw" ]; then
stateok=1
file="./status/status_fw.txt"

if [ -f "$file" ]; then
pyfw=$(echo "$(cat $file)")
else
pywfs="1"
fi

menuname="MOVE FW\n
  FW current position : $pyfw\n"

ls




dialog --title "AO loop configuration" --ok-label "Move" --cancel-label "Back" --help-button --help-label "Exit" --default-item "${pyfw}" --menu "$menuname"  50 80 100 "${pyfwlist[@]}"  2> $tempfile
#sleep 2

retval=$?
choiceval=$(cat $tempfile)
case $retval in
   0) state="menupyfw"
pyfw=$(cat $tempfile)
echo "setFW ${choiceval}"
aoconflog "set FW ${choiceval}"
SetFW ${choiceval}
;; # button 1:
   1) state="menualign";; 
   2) state="menuexit";;
   255) state="menuexit";;
esac
state="menualign"
fi







if [ $state = "menuexit" ]; then
stateok=1
echo "menuexit -> exit"
exit
fi



if [ $stateok = 0 ]; then
echo "state \"$state\" not recognized ... exit"
exit
fi




done
