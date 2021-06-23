#! /bin/bash

user=$1

multi=0
optim=0
golf=0

shopt -s globstar

for g in **/*.tsv; do
	name=$(basename $g .tsv)
	header=$(head -n1 $g)
	kind=$(expr match "$header" '.*#\([^,]*\),')
	count=$(head -n1 $g | grep -o 'count=[0-9]*' | cut -d= -f 2)
	rank=$(grep -F -n $'\t'"$user"$'\t' $g | cut -d: -f 1)
	[ -z "$rank" ] && continue
	(( rank-- ))
	
	case "$kind" in
		multi)
			#set -x
			base=$(( count > 500 ? 5000 : (count * 10) ))
			points=$(echo $rank $count $base | awk '{ print $3^(( $2 - $1 + 1) / $2); }' )
			multi=$(echo $multi $points | awk '{ print $1 + $2 }')
			set +x
			;;
		optim)
			# Adjust the rank for ex-aequos 
			score=$(sed -n "$(( rank + 1 ))p" $g | cut -f 1)
			equals=$(head -n $rank $g | grep "^$score	"|wc -l)
			(( rank -= equals ))
			#set -x
			base=$(( count > 500 ? 2500 : (count * 5) ))
			points=$(echo $rank $count $base | awk '{ print $3^(( $2 - $1 + 1) / $2); }' )
			optim=$(echo $optim $points | awk '{ print $1 + $2 }')
			set +x
			;;
		golf)
			lang=$(basename $g .tsv)
			name=$(basename $(dirname $g))
			# Adjust the rank for ex-aequos 
			score=$(sed -n "$(( rank + 1 ))p" $g | cut -f 1)
			equals=$(head -n $rank $g | grep "^$score	"|wc -l)
			(( rank -= equals ))
			#set -x
			base=200
			points=$(echo $rank $count $base | awk '{ print $3^(( $2 - $1 + 1) / $2); }' )
			golf=$(echo $golf $points | awk '{ print $1 + $2 }')
			set +x
			;;
	esac
	printf "$kind\t$name\t% 4d /% 6d\t=> % 9.3f\n" $rank $count $points
done

total=$(echo $multi $optim $golf | awk '{ print $1 + $2 + $3}' )

printf "multi: % 11.3f\n" $multi
printf "optim: % 11.3f\n" $optim
printf "golf : % 11.3f\n" $golf
printf "total: % 11.3f\n" $total
