#Importing required modules
import matplotlib.pyplot as plt
import numpy as np
import sys

data_file = "kmeans-results.txt"
if(len(sys.argv)>1):
    data_file = sys.argv[1]
#Load Data
data = np.loadtxt(data_file, usecols=(0,1), dtype=np.float32)
#print(data) 

k = 9 # number of clusters
#Load the computed cluster numbers
cluster = np.loadtxt(data_file, usecols=(2), dtype=np.float32)

#Plot the results:
for i in range(k):
    plt.scatter(data[cluster == i , 0] , data[cluster == i , 1] , label = i)
plt.show()
