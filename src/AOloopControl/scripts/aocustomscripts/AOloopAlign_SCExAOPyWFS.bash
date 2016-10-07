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

stringcenter "Pyramid TT align"
menuitems=( "1 ->" "\Zb\Zr$string\Zn" )

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
menuitems+=( "2 ->" "\Zb\Zr$string\Zn" )

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
menuitems+=( "3 ->" "\Zb\Zr$string\Zn" )

menuitems+=( "fl" "Flatten DM for pyWFS" )
menuitems+=( "flk" "End flatten DM process" )
menuitems+=( "flz" "Remove flatten DM solution" )
menuitems+=( "fla" "Apply flatten DM solution" )
menuitems+=( "flm" "Monitor DM flatten tmux session" )
menuitems+=( "" "" )

stringcenter "FILTERS"
menuitems+=( "4 ->" "\Zb\Zr$string\Zn" )

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
pywfs reimage x home
pywfs reimage x goto $pywfsreimagexposref #150000
pywfs reimage y home
pywfs reimage y goto $pywfsreimageyposref #67000
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
pywfs reimage x goto $pywfsreimagexposref #150000
aoconflog "Pupil move x ${pywfsreimagexposref}"
echo "$pywfsreimagexposref $pywfsreimageyposref" > ./status/pcampos.txt
menualign_default="pxm"
state="menualign"
;;
        pxp)
pywfsreimagexposref=$(($pywfsreimagexposref+$pcamstep))
pywfs reimage x goto $pywfsreimagexposref #150000
aoconflog "Pupil move x ${pywfsreimagexposref}"
echo "$pywfsreimagexposref $pywfsreimageyposref" > ./status/pcampos.txt
menualign_default="pxp"
state="menualign"
;;
        pym)
pywfsreimageyposref=$(($pywfsreimageyposref-$pcamstep))
pywfs reimage y goto $pywfsreimageyposref #150000
aoconflog "Pupil move y ${pywfsreimagexposref}"
echo "$pywfsreimagexposref $pywfsreimageyposref" > ./status/pcampos.txt
menualign_default="pym"
state="menualign"
;;
        pyp)
pywfsreimageyposref=$(($pywfsreimageyposref+$pcamstep))
pywfs reimage y goto $pywfsreimageyposref #150000
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
