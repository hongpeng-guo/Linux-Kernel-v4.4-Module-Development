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
        DATAPATH1 = "profile3_1.data"
        DATAPATH2 = "profile3_2.data"
        DATAPATH3 = "profile3_3.data"
        CURDIR = os.path.dirname(os.path.realpath(__file__))
        DATAPATH1 = os.path.join(CURDIR, DATAPATH1)
        DATAPATH2 = os.path.join(CURDIR, DATAPATH2)
        DATAPATH3 = os.path.join(CURDIR, DATAPATH3)

        data1 = np.loadtxt(DATAPATH1)
        data2 = np.loadtxt(DATAPATH2)
        data3 = np.loadtxt(DATAPATH3)

        t = [0]
        cpu1 = [0]
        minf1 = [0]

        cpu2 = [0]
        minf2 = [0]

        cpu3 = [0]
        minf3 = [0]
        
        j = 1
                
        for i in range(data1.shape[0] - 5):
            t.append(0.05 * j)
            cpu1.append(data1[i, 3])
            cpu2.append(np.sum(data2[5*i:5*i+5, 3]))
            cpu3.append(np.sum(data3[11*i:11*i+11, 3]))

            minf1.append(minf1[-1] + data1[i, 1])
            minf2.append(minf2[-1] + np.sum(data2[5*i:5*i+5, 1]))
            minf3.append(minf3[-1] + np.sum(data3[11*i: 11*i+11, 1]))
            j += 1


        fig, ax = plt.subplots()

        line1 = ax.plot(t, cpu1, 'b-')
        line2 = ax.plot(t, cpu2, 'r-')
        line3 = ax.plot(t, cpu3, 'g-')

        ax.set_ylabel('CPU Utilization (%)')
        ax.set_xlabel('Time (sec)')
        ax.legend((line1[0], line2[0], line3[0]), ('200M, 10000 Random, N=1', '200M, 10000 Random N=5', '200M, 10000 Random, N=11'))

        plt.savefig('cpu3.png', format = 'png')
        plt.show()


        fig, ax = plt.subplots()

        line1 = ax.plot(t, minf1, 'b-')
        line2 = ax.plot(t, minf2, 'r-')
        line3 = ax.plot(t, minf3, 'g-')
        
        ax.set_ylabel('Cumulated Minor Page Faults')
        ax.set_xlabel('Time (sec)')
        ax.legend((line1[0], line2[0], line3[0]), ('200M, 10000 Random, N=1', '200M, 10000 Random N=5', '200M, 10000 Random, N=11'))

        plt.savefig('minf3.png', format = 'png')
        plt.show()

if __name__ == "__main__":
    p = Plotter()
    p.plot()

