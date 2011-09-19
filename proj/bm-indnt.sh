#!/bin/sh
for file in src/*.[ch] src/allegro/*.[ch] src/sdl/*.[ch] src/headless/*.[ch]
do
	echo "Indenting $file..."
	
	# Indent style -- This is close but is not perfect
	indent -T bool_t -T int8_t -T int16_t -T int32_t -T int64_t -T uint8_t -T uint16_t -T uint32_t -T uint64_t -T tic_t -T mobj_t \
		-T player_t -T patch_t -T pic_t\
		-kr -bad -bap -bbb -bl -bli0 -cdw -cli4 -cbi4 -nss -npcs -ncs -nbs -saf -sai -saw -nprs\
		-di1 -nbc -nbfda -nbfde -npsl -bls -blf \
		-i4 -lp -ts4 -nlps -il 0 -l160 -nbbo -hnl   \
		"$file"
	
	# Artistic style -- Clear up some formatting issues
	#astyle \
	#	-A1 -t4 -C -S -K -w -Y -m2 -M80 -f -p -H -U -E -xd -y -k1 --mode=c -z2 \
	#	"$file"
	
	astyle \
		-A1 -k1 -t4 -K -S -E -M80 \
		"$file"
	
	#indent -kr -bad -bap -bl -bli0 -nce -cdw -cli4 -cbi0 -nss -npcs -ncs -nprs -saf -sai -nsaw -di1 -nbc -nbfda -nbfde -bls -blf -i4 -lp -l160 -nbbo -ts4 -sob "$file"
done

