count_input = 0

for i in range(28):
	v = (count_input*8+7) % 104
	print(f"[{v} : {v-7}]") 
	if count_input == 12: count_input = 0 	# % 13
	else: count_input += 1