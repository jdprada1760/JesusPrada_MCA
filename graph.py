# Plots histogram for numbers in dist.data
import numpy as np
import matplotlib.pyplot as plt

# Gives the x,y of the histogram for logmasses from the given array of data
def histog( mf, nbins ):
    hist, bin_edges = np.histogram(mf, bins = nbins)
    # Obtains the center point for each bin
    xcenter = (bin_edges[:-1] + bin_edges[1:])/2
    return np.array(xcenter), np.array(hist)

# Loads the file
filename = "dist.data"
data = np.loadtxt(filename)
# Gets x,y points for the histogram with 10 bins
x,y = histog(data,8)
plt.bar(x,y,align='center',width = x[1]-x[0]) # A bar chart
#plt.show()
plt.savefig("graph.png")
