with open("input.txt") as input_file: 
    
    lines = [line.strip().split(',') for line in input_file]
    results = lines
    results = [[int(float(j)) for j in i] for i in results]
    val =  results[results[0][0]][results[0][0]-1]
    
    for j in xrange(results[0][0], 0, -1): #rows
        for i in xrange((results[0][0] - 1), -1, -1):
            #If ( i+1,j+1 > array.length) then keep value, #at the ending pos
            if i+1 > (len(results[results[0][0]]) -1) and j+1 > (len(results) - 1):
                pass#skip we are at the corner
                #print "skipped"
            elif j+1 > (len(results) - 1):
                # we are at the bottow row, no down move
                if results[j][i] < results[j][i+1] + results[j][i]:
                    #update
                    results[j][i] = results[j][i] + results[j][i+1]
                if (val < results[j][i]):
                    val = results[j][i];
            elif i+1 > (len(results[results[0][0]]) -1): # we have no right movei
                #you are at the end.... leave it?
                if results[j][i] < results[j+1][i] + results[j][i]:
                    #update
                    results[j][i] = results[j][i] + results[j+1][i]
                if (val < results[j][i]):
                    val = results[j][i];
            else:
                 #if ((array[j+1][i] +  array[j][i]) > (array[j][i+1] + array[j][i+1])) #down is better
                 if ((results[j+1][i] + results[j][i]) > (results[j][i+1] + results[j][i])):
                     results[j][i] = results[j+1][i] + results[j][i] #down is better
                 elif ((results[j+1][i] + results[j][i]) < (results[j][i+1] + results[j][i])):
                     results[j][i] = results[j][i+1] + results[j][i] #down is better
                 if ((results[j][i] > val)):
                     val = results[j][i]
   



    #print results
    print "val is = ", val

with open("output.txt", "w") as output_file: 
    output_file.write("Hello World") 
    output_file.write("This is our new text output_file") 
    output_file.write("and this is another line.") 
    output_file.write("Why? Because we can.") 
    output_file.close()
