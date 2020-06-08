
import java.awt.Graphics;

import javax.swing.JFrame;
import javax.swing.JPanel;
import java.awt.Color;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

class AppMain
{

    private static JFrame win;
    private static Screen scr;

    public static final int WIDTH = 720;
    public static final int HEIGHT = 480;

    static
    {
        System.loadLibrary("dotcvm");
    }

    private static native int _init();
    private static native void _run(String[] args);
    private static native void _exit();

    public static void main(String[] args)
    {
        int errCode = _init();
        if(errCode == 1) // setPixel Method not found
        {
            System.out.println("Native code couldn't find setPixel(III)V");
            System.exit(1);
        }
        else if(errCode == 2) // refreshScr Method not found
        {
            System.out.println("Native code couldn't find refreshScr()V");
            System.exit(2);
        }
        else if(errCode == 3) // requestClose Method not found
        {
            System.out.println("Native code couldn't find requestClose()V");
            System.exit(3);
        }
        else    // Everything's okay.
        {
            scr = new Screen(WIDTH, HEIGHT);
            win = createWindow(WIDTH, HEIGHT);
            _run(args); // Jump to native code
        }
    }

    public static void setPixel(int x, int y, int color)
    {
        scr.setPixel(x, y, color);
    }

    public static void refreshScr()
    {
        win.getContentPane().repaint();
    }

    public static void requestClose()
    {
        win.dispatchEvent(new WindowEvent(win, WindowEvent.WINDOW_CLOSING));
    }

    private static JFrame createWindow(int width, int height)
    {
        JFrame frame = new JFrame();
        frame.setTitle("DotC VirtualMachine");
        frame.setResizable(false);
        frame.setSize(width, height);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLocationRelativeTo(null);
        frame.addWindowListener(new WindowAdapter() 
        {
            public void windowClosing(WindowEvent e) {
                _exit();
                System.exit(0);
            }
        });
        frame.setVisible(true);

        frame.setContentPane(new JPanel()
        {
            private static final long serialVersionUID = 1L;

            @Override
            protected void paintComponent(Graphics g) 
            {
                scr.refresh(g);
            }
        });

        return frame;
    }

    private static class Screen
    {

        int[][] pixels;

        public Screen(int width, int height)
        {
            pixels = new int[width][height];
            for(int x = 0; x < width; x++)
                for(int y = 0; y < height; y++)
                    pixels[x][y] = 0;
        }


        public void setPixel(int x, int y, int color)
        {
            pixels[x][y] = color;
        }

        public void refresh(Graphics g)
        {
            for(int x = 0; x < pixels.length; x++)
                for(int y = 0; y < pixels[x].length; y++)
                {
                    g.setColor(new Color(pixels[x][y]));
                    g.fillRect(x, y, 1, 2);
                }
        }



    }

}
