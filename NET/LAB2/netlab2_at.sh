######################################
# Eltex net lab #2 - alarm
# Student: Alexey Leushin
# ========================
# $1 - command
# $2 - argument for comand (optional)
######################################

# Fails:
#	fail1: /bin/sh -> bash
#	fail2: clementine -> paplay

##########################
# === functions ===
##########################

# Loads current job list form file
# args:
# 	$1 - full job file name
update_job_list()
{
	source $1
}

# Plays sound
# args:
#	$1 - full sound name
playsnd()
{
	# clementine sucks
	 #clementine -l $1 -p &
	
	paplay $1 &
}

# Shows at jobs
jobs_at_show()
{
	atq
}

# Clears at jobs
jobs_at_clear()
{
	# source: https://unix.stackexchange.com/questions/53144/remove-all-at-jobs
	for i in `atq | awk '{print $1}'`
	do
		atrm $i
	done
}

# Shows alarm list
# args:
#	$1 - full jobs file name
jobs_show()
{
	# How to work with arrays:
	# for i in ${arr[*]}; do echo $i; done
	
	# Get job vars and arrays from file
	update_job_list $1
	
	# Show number of planned jobs
	echo " "
	echo "There are $NJOBS job(s) in the schedule"
	
	# Show info on each job
	echo " "
	i=0
	while [ $i -lt $NJOBS ]
	do
		echo "=== Job #$(($i+1)) ==="
		echo "Time: ${TIMS[i]}"
		echo "Job:  ${JOBS[i]}"
		echo " "
		i=$(($i+1))
	done
	echo " "
}

# Syncs jobfile with at
# args:
#	$1 - job file to be synced with at
jobs_at_sync()
{
	# Clear old schedule
	# source: https://unix.stackexchange.com/questions/53144/remove-all-at-jobs
	for i in `atq | awk '{print $1}'`
	do
		atrm $i
	done
	
	# Get new schedule from job file
	update_job_list $1
	
	# Add new schedule
	i=0
	while [ $i -lt $NJOBS ]
	do
		AT_CMD=${TIMS[$i]}
		AT_INP=${JOBS[$i]}
		
		echo "at $AT_CMD"
		
		#https://stackoverflow.com/questions/15454199/how-can-i-use-an-at-command-in-a-shell-script
		at $AT_CMD <<< "$AT_INP"
		
		i=$(($i+1))
	done
}



##########################
# === main code ===
##########################

# Get work dirs and files
export PROG=$0
export WDIR=$( dirname "$0" )/netlab2
export JFILE=$WDIR/jobs_list.txt
export LOG=$WDIR/log.txt

# Parse and execute command
if [ "$1" == "help" ]
then
	# === Print help === #
	echo "Place script in one of folders specified in PATH envar"
	echo "After that you can type: [script_name] [cmd]"
	echo " [cmd]:"
	echo "  wdir - show working directory (place alarm sounds here)"
	echo "  add/show/sync/clear - job list manipulations"
	echo "  rem [n] - remove job #n from the job list"
	echo "  schedule - check out at schedule"
	echo "  play [file] - play specified file (test sound playback)"
	echo "  shlog/clog - log file manipulations"
	echo " "
	exit 0 # don't log this
elif [ "$1" == "wdir" ]
then
	# === Print dirs === #
	echo "Workdir: $WDIR"
	exit 0 # don't log this
elif [ "$1" == "show" ]
then
	# === Show job list === #
	jobs_show $JFILE
	exit 0 # don't log this
elif [ "$1" == "schedule" ]
then
	# === Show at schedule === #
	jobs_at_show
	exit 0 # don't log this
elif [ "$1" == "add" ]
then
	# === Add job === #
	
	# Get time
	echo ""
	echo "=== Creating new job ==="
	echo "Enter year:"
	read YEAR
	echo "Enter month:"
	read MON
	echo "Enter day of month:"
	read DAY
	echo "Enter time [hh:mm]:"
	read TIME
	echo ""
		echo $DAY $MON $YEAR $TIME
	
	# Get soundname
	echo "Available sounds: `ls $WDIR -I "*.txt"`"
	echo "Enter sound name from list above:"
	read SND
	SND=$WDIR/$SND
		echo $SND
	
	# Load job list
	update_job_list $JFILE
	
	# Add job to arrays
	TIMS[NJOBS]="$TIME $MON/$DAY/$YEAR" 
	JOBS[NJOBS]="bash $PROG play $SND"
	NJOBS=$(($NJOBS+1))
		#echo "Num jobs: $NJOBS"
		#echo "Times: ${TIMS[*]}"
		#echo "Jobs: ${JOBS[*]}"
	
	# Dump arrays in job file
	echo "NJOBS=$NJOBS" > $JFILE
	i=0
	while [ $i -lt $NJOBS ]
	do
		echo "#=== Job #$(($i+1)) ===" >> $JFILE
		echo "TIMS[$i]=\"${TIMS[i]}\"" >> $JFILE
		echo "JOBS[$i]=\"${JOBS[i]}\"" >> $JFILE
		i=$(($i+1))
	done
	
	# Sync new schedule
	jobs_at_sync $JFILE
elif [ "$1" == "rem" ]
then
	# === Remove job === #
	echo "Removing job #$2 ..."
	
	# Load job list
	update_job_list $JFILE
	
	# Check if empty
	if [[ ($2 -lt 1 || $2 -gt $NJOBS) ]]
	then
		echo "Invalid job number ..."
		exit 0
	fi
	
	# Decrement job number
	JNUM=$(($2-1))
	
	# Remove job from arrays (concatenate subarrays)
	# source: https://unix.stackexchange.com/questions/393069/how-do-i-shift-a-bash-array-at-some-index-in-the-middle
	TIMS=("${TIMS[@]:0:$JNUM}" "${TIMS[@]:$(($JNUM+1))}")
	JOBS=("${JOBS[@]:0:$JNUM}" "${JOBS[@]:$(($JNUM+1))}")
	NJOBS=$(($NJOBS-1))
	
	# Dump arrays in job file
	echo "NJOBS=$NJOBS" > $JFILE
	i=0
	while [ $i -lt $NJOBS ]
	do
		echo "#=== Job #$(($i+1)) ===" >> $JFILE
		echo "TIMS[$i]=\"${TIMS[i]}\"" >> $JFILE
		echo "JOBS[$i]=\"${JOBS[i]}\"" >> $JFILE
		i=$(($i+1))
	done
	
	# Sync new schedule
	jobs_at_sync $JFILE
elif [ "$1" == "play" ]
then
	# === Sync schedule === #
	playsnd $2
elif [ "$1" == "sync" ]
then
	# === Sync schedule === #
	jobs_at_sync $JFILE
elif [ "$1" == "clear" ]
then
	# === Clear all jobs === #
	jobs_at_clear
	echo "NJOBS=0" > $WDIR/jobs_list.txt
elif [ "$1" == "shlog" ]
then
	# === Show log === #
	cat $LOG
	exit 0 # don't log this
elif [ "$1" == "clog" ]
then
	# === Clean log === #
	echo " " > $LOG
elif [ "$1" == "test" ]
then
	# === Test === #
	echo "TEST"
else
	# === Unknown comand === #
	echo "Unknown command, use \"help\" parameter to see comands"
fi

# Add log entry
echo "(`date`) $1 $2 $3 $4" >> $LOG
