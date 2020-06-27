
import java.awt.Graphics;

import javax.swing.JFrame;
import javax.swing.JPanel;
import java.awt.Color;
import java.awt.event.WindowEvent;

class AppMain
{

    private static JFrame win;
    private static Screen scr;

    public static final int WIDTH = 720;
    public static final int HEIGHT = 480;

    static
    {
        System.loadLibrary("DotCVM");
    }

    private static native int _init();
    private static native void _run(String[] args);
    private static native void _update();
    private static native void _exit();

    public static void main(final String[] args) {
        final int errCode = _init();
        if (errCode == 1) // setPixel Method not found
        {
            System.out.println("Native code couldn't find setPixel(III)V");
            System.exit(1);
        } else if (errCode == 2) // refreshScr Method not found
        {
            System.out.println("Native code couldn't find refreshScr()V");
            System.exit(2);
        } else if (errCode == 3) // requestClose Method not found
        {
            System.out.println("Native code couldn't find requestClose()V");
            System.exit(3);
        } else if (errCode == 4) // getPixel Method not found
        {
            System.out.println("Native code couldn't find getPixel(II)I");
            System.exit(4);
        } else // Everything's okay.
        {
            scr = new Screen(WIDTH, HEIGHT);
            win = createWindow(WIDTH, HEIGHT);
            _run(args); // Start native code
            while (win.isVisible())
                _update();
            win.setTitle("Wait while Shutting down the machine");
            win.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
            win.setVisible(true);
            _exit();
            win.setVisible(false);
            System.exit(0);
        }
    }

    public static void setPixel(final int x, final int y, final int color) {
        scr.setPixel(x, y, color);
    }

    public static int getPixel(final int x, final int y) {
        return scr.getPixel(x, y);
    }

    public static void refreshScr() {
        win.getContentPane().repaint();
    }

    public static void requestClose() {
        win.dispatchEvent(new WindowEvent(win, WindowEvent.WINDOW_CLOSING));
    }

    private static JFrame createWindow(final int width, final int height) {
        final JFrame frame = new JFrame();
        frame.setTitle("DotC VirtualMachine");
        frame.setResizable(false);
        frame.setSize(width, height);
        frame.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);

        frame.setContentPane(new JPanel() {
            private static final long serialVersionUID = 1L;

            @Override
            protected void paintComponent(final Graphics g) {
                scr.refresh(g);
            }
        });

        return frame;
    }

    private static class Screen {

        int[][] pixels;

        public Screen(final int width, final int height) {
            pixels = new int[width][height];
            for (int x = 0; x < width; x++)
                for (int y = 0; y < height; y++)
                    pixels[x][y] = 0;
        }

        public void setPixel(final int x, final int y, final int color) {
            pixels[x][y] = color;
        }

        public int getPixel(final int x, final int y) {
            return pixels[x][y];
        }

        public void refresh(final Graphics g)
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
