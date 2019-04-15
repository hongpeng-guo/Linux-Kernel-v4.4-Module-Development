import numpy as np
import matplotlib
import os
#matplotlib.use('Agg')

import matplotlib.pyplot as plt

class Plotter(object):

    """Docstring for PlotterImpl. """

    def __init__(self):
        """TODO: to be defined1. """

    def plot(self):
        """TODO: Docstring for plot.
        :returns: TODO

        """
        DATAPATH = "profile1.data"
        CURDIR = os.path.dirname(os.path.realpath(__file__))
        DATAPATH = os.path.join(CURDIR, DATAPATH)

        print(DATAPATH)
        data = np.loadtxt(DATAPATH)

        t1 = [0]
        cpu1 = [0]
        minf1 = [0]

        t2 = [0]
        cpu2 = [0]
        minf2 = [0]

        j = 1
        k = 1
        for i in range(data.shape[0]):
            if int(data[i][0]) == 3428:
                t1.append(0.05 * j)
                cpu1.append(data[i][3])
                minf1.append(minf1[-1] + data[i][1])
                j += 1
            else:
                t2.append(0.05 * k)
                cpu2.append(data[i][3])
                minf2.append(minf2[-1] + data[i][1])
                k += 1


        fig, ax = plt.subplots()

        line1 = ax.plot(t1, cpu1, 'r-')
        line2 = ax.plot(t2, cpu2, 'b-')

        ax.set_ylabel('CPU Utilization (%)')
        ax.set_xlabel('Time (sec)')
        ax.legend((line1[0], line2[0]), ('1024M, 10000 Random', '1024M, 50000 Random'))

        plt.savefig('cpu1.png', format = 'png')
        plt.show()


        fig, ax = plt.subplots()

        line1 = ax.plot(t1, minf1, 'r-')
        line2 = ax.plot(t2, minf2, 'b-')

        ax.set_ylabel('Cumulated Minor Page Faults')
        ax.set_xlabel('Time (sec)')
        ax.legend((line1[0], line2[0]), ('1024M, 10000 Random', '1024M, 50000 Random'))

        plt.savefig('minf1.png', format = 'png')
        plt.show()


if __name__ == "__main__":
    p = Plotter()
    p.plot()

