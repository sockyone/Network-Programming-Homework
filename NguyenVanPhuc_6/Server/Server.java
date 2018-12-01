import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;

/**
 *
 * @author phuc
 */
public class Server {

    public final static int BUFFER_SIZE = 16 * 1024;
    public final static int PORT = 3001;
    public static int nFiles = 0;

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        try {
            ServerSocket serverSocket = new ServerSocket(PORT);

            while (true) {
                Socket socket = serverSocket.accept();
                System.out.println("Address: " + socket.getInetAddress() + ":" + socket.getPort());
                Thread multipleThread = new MultipleThread(socket);
                multipleThread.start();
            }
        } catch (Exception e) {
            System.out.println(e);
        }
    }

}

class MultipleThread extends Thread {

    public final int BUFFER_SIZE = 16 * 1024;
    private Socket socket;

    public MultipleThread(Socket socket) {
        this.socket = socket;
    }

    public void run() {
        InputStream is = null;
        try {
            is = this.socket.getInputStream();
        } catch (IOException ex) {
            System.out.println(ex);
        }
        InputStreamReader isr = new InputStreamReader(is);
        BufferedReader br = new BufferedReader(isr);

        while (true) {
            String fileName = null;
            OutputStream os = null;
            try {
                fileName = br.readLine();
                os = socket.getOutputStream();
            } catch (IOException ex) {
                ex.printStackTrace();
            }

            if (fileName == null) {
                break;
            }
            System.out.println("File name: " + fileName);
            DataOutputStream dos = new DataOutputStream(os);
            long fileSize = 0;

            try {
                //Get file
                String workDir = System.getProperty("user.dir");
                File file = null;
                file = new File(workDir + "/" + fileName);

                // Get file size
                fileSize = file.length();
                dos.writeLong((long) fileSize);
                if (fileSize <= 0) {
                    System.out.println("File not found");
                    continue;
                }

                // Send content
                FileInputStream fis = new FileInputStream(file);
                byte fileContent[] = new byte[BUFFER_SIZE];
                int count;
                while ((count = fis.read(fileContent)) > 0) {
                    os.write(fileContent, 0, count);
                }
                os.flush();
                fis.close();
                increaseFiles();
            } catch (Exception e) {
                System.out.println(e);
            }
        }

        try {
            socket.close();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    public synchronized void increaseFiles() throws Exception {
        Server.nFiles++;
        System.out.println("Total files: " + Server.nFiles);
    }
}
