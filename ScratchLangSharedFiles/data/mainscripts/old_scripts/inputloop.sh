#!/bin/bash
if [ -f var/pe ]; then
  getnextp() {
    res=0
    small=$1
    execute=
    while :; do
      ((res++))
      if [ "$res" -gt "$(sed -n '$=' plist)" ]; then
        break
      fi
      reser=$(sed "$res"'!d' plist)
      if [ "$reser" == "$0" ]; then
        ((res++))
        ((res++))
        reser=$(sed "$res"'!d' plist)
        if [ "$reser" -lt "$small" ]; then
          small="$reser"
          ((res--))
          reser=$(sed "$res"'!d' plist)
          execute=$reser
          ((res++))
        fi
      fi
    done
    don=2
  }
  npl=999999999
  don=1
  step() {
    if [ -d var ]; then
      if [[ "$(cat plist)" == *"$0"* ]]; then
        if [ "$don" == 1 ]; then
          getnextp "$1"
        fi
        if [ "${BASH_LINENO[0]}" == "$small" ]; then
          cd ../
          ./packages/"$execute"
          cd mainscripts || exit
          don=1
        fi
      fi
    fi
  }
  trap 'step $npl' DEBUG
fi
RED='\033[0;31m'
NC='\033[0m'
echo
if [ h"$1" == h ]; then
  echo "1. Create a project."
  echo "2. Remove a project."
  echo "3. Compile a project."
  echo "4. Decompile a .sb3 file."
  echo "5. Export project."
  echo "6. Import project."
  echo "7. Are options 3 and 4 not working? Input 7 to install dependencies."
  echo "8. Create scratchlang command."
  echo "9. Remove scratchlang command."
  echo "A. Enable Developer Mode."
  echo "B. Manage Packages"
  echo "C. Exit."
  read -rsn 1 input
else #if there's arguments, set input to the arg
  case $1 in
  -1)
    input=1
    ;;
  -2)
    input=2
    ;;
  -3)
    input=3
    ;;
  -4)
    input=4
    ;;
  -5)
    input=5
    ;;
  -6)
    input=6
    ;;
  esac
  if [ "h$input" == h ]; then
    echo -e "${RED}Error: $1 is not an argument.${NC}"
    sleep 2
    ./start.sh nope #make sure not to check the version when running ./start.sh again
    exit
  fi
fi
if [ h$input == h1 ]; then
  dir=$PWD
  echo
  echo "Name your project. Keep in mind that it cannot be empty or it will not be created properly." #Name you project
  read -r name
  cd ../
  if [ "h$name" == h ]; then
    echo -e "${RED}Error: Project name empty.${NC}"
    exit
  elif [ -d projects/"$name" ]; then
    echo "Project $name already exists. Replace? [Y/N]"
    read -rsn 1 yessor
    if [ h"$yessor" == hy ] || [ h"$yessor" == hY ]; then
      rm -rf projects/"$name"
    elif [ h"$yessor" == hn ] || [ h"$yessor" == hN ]; then
      exit
    else
      echo -e "${RED}Error: $yessor is not an input.${NC}"
      exit
    fi
  fi
  echo "You named your project $name. If you want to rename it, use the File Explorer."
  if ! [ -d projects ]; then
    mkdir projects
  fi
  cd projects || exit
  mkdir "$name"
  cd "$name" || exit                               #go to project dir, create a folder named whatever you named it, and go into that
  echo >>.maindir "Please don't remove this file." #tells the compiler that it's in the right directory
  mkdir Stage
  cd Stage || exit
  mkdir assets
  cd ../
  cd ../
  cd ../
  cp resources/cd21514d0531fdffb22204e0ec5ed84a.svg projects/"$name"/Stage/assets #Copy blank background svg to Stage/assets
  cd projects/"$name"/Stage || exit
  echo >>project.ss1 "# There should be no empty lines." #create .ss file
  echo >>project.ss1 ss
  cd ../
  mkdir Sprite1 #create sprite 1
  cd Sprite1 || exit
  echo >>project.ss1 "# There should be no empty lines."
  echo >>project.ss1 ss
  mkdir assets
  cd ../
  cd ../
  cd ../
  cp resources/341ff8639e74404142c11ad52929b021.svg projects/"$name"/Sprite1/assets #copy scratch cat sprites to the assets folder
  cp resources/c9466893cdbdda41de3ec986256e5a47.svg projects/"$name"/Sprite1/assets
  cd mainscripts || exit
elif [ h$input == h2 ]; then
  cd ../
  if ! [ -d projects ]; then
    echo -e "${RED}Error: there are no projects to delete.${NC}"
    exit
  fi
  cd projects || exit
  echo
  ls -1
  echo
  echo "Choose a project to get rid of, or input nothing to cancel."
  read -r pgrd
  if ! [ h"$pgrd" == h ]; then
    if [ -d "$pgrd" ]; then
      rm -rf "$pgrd"
    else
      echo -e "${RED}Error: directory $pgrd does not exist.${NC}"
    fi
  fi
elif [ h$input == h3 ]; then
  chmod 755 compiler.v1.ss1.sh
  ./compiler.v1.ss1.sh
elif [ h$input == h4 ]; then
  if [ -f var/ds ]; then
    if [ "$(sed '1!d' var/ds)" == "py" ]; then
      python3 decompilerpy.v1.ss1.py
    else
      chmod 755 "$(sed '1!d' var/ds)" #if there is a custom compiler, get the command from the ds file and run it
      ./"$(sed '1!d' var/ds)"
    fi
  else
    chmod 755 decompiler.v2.ss1.sh
    ./decompiler.v2.ss1.sh
  fi
elif [ h$input == h5 ]; then
  cd ../
  if ! [ -d projects ]; then
    echo -e "${RED}Error: there are no projects to export.${NC}"
    exit
  fi
  cd projects || exit
  echo
  ls -1
  echo
  echo "Choose a project to export, or input nothing to cancel."
  read -r pgrd
  if ! [ h"$pgrd" == h ]; then
    if [ -d "$pgrd" ]; then
      tar -cf "$pgrd".ssa "$pgrd" #export project as a ScratchScript Archive
      cd ../
      cp projects/"$pgrd".ssa exports
      rm projects/"$pgrd".ssa
      echo "Your project $pgrd.ssa can be found in the exports folder."
    else
      echo -e "${RED}Error: directory $pgrd does not exist.${NC}"
    fi
  fi
elif [ h$input == h6 ]; then
  if ! [ -f var/zenity ]; then
    echo "Do you have the command zenity? [Y/N]" #Ask user if they have the command zenity
    read -rsn 1 input3
  else
    input3=y
  fi
  if [ h$input3 == hY ] || [ h$input3 == hy ]; then
    import=$(zenity -file-selection -file-filter 'ScratchScript\ Archive *.ssa') #Look for ScratchScript Archives
    cd ../
    if ! [ -d projects ]; then
      mkdir projects
    fi
    cd projects || exit
    tar -xf "$import"              #extract .ssa archive
    echo "Remove .ssa file? [Y/N]" #remove the .ssa archive if wanted
    read -rsn 1 f
    if [ h"$f" == hY ] || [ h"$f" == hy ]; then
      rm "$import"
    fi
  elif [ h$input3 == hN ] || [ h$input3 == hn ]; then
    echo
  else
    echo "${RED}Error: $input3 not an input.${NC}"
  fi
elif [ h$input == h7 ]; then
  echo "This only works for MSYS2/MINGW. Continue? [Y/N]"
  read -rsn 1 con
  if [ h"$con" == hY ] || [ h"$con" == hy ]; then #Start installing dependencies for mingw
    echo
    pacman -S --noconfirm unzip
    bit=$(getconf LONG_BIT) #get bit number (32, 64) of PC
    dir=$PWD
    cd || exit
    mkdir zenity #start installing zenity
    cd zenity || exit
    if [ "$bit" == 64 ]; then
      wget https://github.com/ncruces/zenity/releases/download/v0.9.0/zenity_win64.zip #get zenity64
      version=zenity_win64
    else
      wget https://github.com/ncruces/zenity/releases/download/v0.9.0/zenity_win32.zip #get zenity 32
      version=zenity_win32
    fi
    unzip -n $version.zip
    cp zenity.exe /usr/bin #copy zenity to /usr/bin and remove original exe
    cd ../
    rm -rf zenity
    cd "$dir" || exit
    pacman --noconfirm -S git #install git
    pacman --noconfirm -S bc  #install bc
    ./start.sh nope
  elif [ h"$con" == hN ] || [ h"$con" == hn ]; then
    echo
  else
    echo -e "${RED}Error: $con not an input.${NC}"
  fi
elif [ h$input == h8 ]; then
  dir=$PWD
  if ! [ -f var/alias ]; then #create scratchlang command
    echo >>/usr/bin/scratchlang "cd $dir && ./start.sh \$1 \$2 \$3"
    echo >>var/alias "This file tells the program that the command is already created. Please don't touch this."
  else
    echo "scratchlang command has already been created."
  fi
elif [ h$input == h9 ]; then #scratchlang command removal loop
  chmod 755 rmaliasiloop.sh
  ./rmaliasiloop.sh
elif [ h$input == hA ] || [ h$input == ha ]; then #enable devmode
  echo >>var/devmode "This is a devmode file. You can manually remove it to disable dev mode if you don't want to use the program to disable it for some reason."
  ./start.sh nope
elif [ h$input == hB ] || [ h$input == hb ]; then
  chmod 755 pmloop.sh
  ./pmloop.sh
elif [ h$input == hC ] || [ h$input == hc ]; then
  clear
else
  echo -e "${RED}Error: $input is not an input.${NC}"
  ./inputloop.sh
fi
