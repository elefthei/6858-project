#!/bin/bash
./make_config.sh
sed -i 's/USERGROUPS_ENAB\ *YES/USERGROUPS_ENAB\ \ \ \ NO/g' /etc/login.defs #changes USERGROUPS_ENAB so NO new groups are created automatically