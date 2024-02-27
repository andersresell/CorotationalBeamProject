import numpy as np
import matplotlib.pyplot as plt
import os
from mpl_toolkits.mplot3d import Axes3D

# fmt: off

#need this to make ctrl+c close plot window
import signal
signal.signal(signal.SIGINT, signal.SIG_DFL)

SMALL_VAL = 1e-6
MAX_TRIADS = 6
MAX_NODES=1000

def plot_triad(ax, x,U,scale=1):
    assert x.shape==(3,)
    assert U.shape==(3,3)
    U*=scale
    ax.quiver(x[0],x[1],x[2],U[0,0],U[1,0],U[2,0],color="r")
    ax.quiver(x[0],x[1],x[2],U[0,1],U[1,1],U[2,1],color="g")
    ax.quiver(x[0],x[1],x[2],U[0,2],U[1,2],U[2,2],color="b")


class Plotter:
    
    def __init__(self,simdir,write_gif=True) -> None:
        os.chdir(simdir)
        print("Python script running from the following directory:\n",os.getcwd())
        self.output_dir = os.path.join(os.getcwd(),"output")
        header = self.read_header(0)
        self.N = int(header[0])
        data = self.read_data(0)
        X = data[:,0:3]
        self.L0 =  X[-1][0] - X[0][0]
        self.n_steps = int(header[1])
        self.n_write = int(header[2])
        self.n_plot_triad = max(int(self.N/MAX_TRIADS),1)
        self.write_gif=write_gif
        if write_gif:
            self.output_tmp = "output_tmp"
            if os.path.exists(self.output_tmp) is False:
                os.makedirs(self.output_tmp)
        if self.N>MAX_NODES:
            node_stride = int(self.N/MAX_NODES)
            self.i_nodes = np.arange(0,self.N,node_stride)
        else:
            self.i_nodes = np.arange(0,self.N,1)
    def read_header(self,n):
        return np.genfromtxt(os.path.join(self.output_dir, str(n)+".csv"), delimiter=",", skip_header=1,max_rows=1)
    def read_data(self,n):
        return  np.genfromtxt(os.path.join(self.output_dir, str(n) + ".csv"), delimiter=",", skip_header=4)
    
    def read_header_transient(self,n):
        header = self.read_header(n)
        self.t = header[3]
        self.dt = header[4]
        
    def animate_3d(self):
        ax = plt.figure(figsize=(8,6)).add_subplot(111,projection="3d")
        for n in range(self.n_steps):
            if n % self.n_write != 0: continue
            self.read_header_transient(n)
            
            data = self.read_data(n)
            X = data[:,0:3]
            
            quiver_scale = self.L0/10
            u = data[:,3:6]
            x = X+u
            assert self.N==x.shape[0]
            triads = data[:,6:15]
            
            ax.cla()
            ax.plot3D(x[self.i_nodes,0],x[self.i_nodes,1],x[self.i_nodes,2],".-",color="black")
            
            for i in range(self.N):
                if i%self.n_plot_triad!=0: continue
                assert triads[i,:].shape == (9,)
                U =  triads[i,:].reshape(3,3)
                plot_triad(ax,x[i,:],U,quiver_scale)
                
            plt.axis("equal")
            # title = "n={: <6}, t={: <6}, dt={: <6}".format(n, t, dt)
            # plt.title(title)
           
            plt.title("n="+str(n)+", t="+str(self.t)+", dt="+str(self.dt))
            
            axlen = 1.01*self.L0
            ax.set_xlim(0,axlen)
            ax.set_ylim(-axlen/2,axlen/2)
            ax.set_zlim(-axlen/2,axlen/2)
            ax.set_xlabel("x")
            ax.set_ylabel("y")
            ax.set_zlabel("z")
            ax.set_xlim()   
            if self.write_gif:
                filepath = os.path.join(self.output_tmp, str(n) + ".png")
                plt.savefig(filepath,dpi=200)
            plt.pause(SMALL_VAL)
            
        if self.write_gif: self.create_gif()
    
    def plot_end_node_transient(self):
        
        
        times = []
        
        disp = []
        for n in range(self.n_steps):
  
            if n % self.n_write != 0: continue
            self.read_header_transient(n)
            times.append(self.t)
            
            data = self.read_data(n)
            u = data[:,3:6]
            v =data[:,15:18]
            omega_u = data[:,18:21]
            
            #disp.append(omega_u[-1][2])
            #disp.append(u[-1][0])
            disp.append(v[-1][2])
            
        plt.figure()
        times = np.array(times)
        disp = np.array(disp)
        plt.plot(times,disp)
        plt.xlabel("t")
        plt.xlim([0,5])
        plt.ylim([-5,10])
        
    def animate_vertical_disp(self):
        plt.figure()
        for n in range(self.n_steps):
            if n % self.n_write != 0: continue
            self.read_header_transient(n)
            data = self.read_data(n)
            u3 = data[:,5]
            plt.cla()
            plt.plot(u3,'-')
            plt.xlabel("i")
            plt.ylabel("u3")
            plt.ylim([-self.L0/5,self.L0/5])
            plt.pause(SMALL_VAL)

    
    
    def create_gif(self):
        assert self.write_gif
        print("Writing Gif...")
        from PIL import Image

        image_files = [str(n) + ".png" for n in range(0, self.n_steps, self.n_write)]
        images = []
        for img_file in image_files:
            img = Image.open(os.path.join(self.output_tmp, img_file))
            images.append(img)

        output_name = "beam.gif"
        dt_frame = 1000.0 / 60  # 60 frames per second
        images[0].save(
            output_name, save_all=True, append_images=images[1:], duration=dt_frame, loop=0
        )
        for img in images:
            img.close()
        import shutil
        shutil.rmtree(self.output_tmp)
        print("Gif done")
            

if __name__ == "__main__":

    p = Plotter("testing",write_gif=False)
    p.plot_end_node_transient()
    #p.animate_vertical_disp()  
    p.animate_3d()
    
    plt.show()
