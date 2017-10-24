with open("input.txt") as input_file: 
    #print input_file.read()
    #for line in input_file:
    #    int_list.append([int(i) for i in line.split(',')])
        #print int_list
    lines = [line.strip().split(',') for line in input_file]
    #cintlist = [int(i) for i in lines] 
    print "n is ", lines[0]
    #lines[0] = lines[0] + 1
    #print lines[0]
    results = lines
    results = [[int(float(j)) for j in i] for i in results]
    print results
    print "that was results"
    print "results = ", results[0][0] + 1
    print "results[0].length = ", len(results[0])
    #n = cintlist[0]
    #print "n is ", n
    #print "testing integer arith on strings?", (n + 1)

with open("output.txt", "w") as output_file: 
    output_file.write("Hello World") 
    output_file.write("This is our new text output_file") 
    output_file.write("and this is another line.") 
    output_file.write("Why? Because we can.") 
    output_file.close()
