﻿using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using System.Windows.Forms;

namespace network.loki.lokinet.win32.ui
{
    public partial class dlgBootstrap : Form
    {
        public dlgBootstrap()
        {
            InitializeComponent();
            if (Program.platform == PlatformID.Win32NT)
                default_path = Environment.ExpandEnvironmentVariables("%APPDATA%\\.lokinet\\bootstrap.signed");
            else
                default_path = Environment.ExpandEnvironmentVariables("%HOME%/.lokinet/bootstrap.signed");
            label2.Text = String.Format("This file is automatically saved as {0}.", default_path);
        }

        private WebClient wc;
        private string default_path;
        private void button1_Click(object sender, EventArgs e)
        {
            ServicePointManager.ServerCertificateValidationCallback += cert_check;
            ServicePointManager.SecurityProtocol = (SecurityProtocolType)48 | 0 | (SecurityProtocolType)192 | (SecurityProtocolType)768 | (SecurityProtocolType)3072;
            wc = new WebClient();
            // add something more unique, this is the IE 5.0 default string
            wc.Headers.Add("User-Agent","Mozilla/4.0 (compatible; MSIE 5.0; Windows NT 5.0)");
            try
            {
                wc.DownloadFile(uriBox.Text, default_path);
                MessageBox.Show("LokiNET node bootstrapped", "LokiNET", MessageBoxButtons.OK, MessageBoxIcon.Information);
                DialogResult = DialogResult.OK;
            }
            catch (NotSupportedException ex)
            {
                string lokinetExeString;
                Process lokinet_bootstrap = new Process();

                if (Program.platform == PlatformID.Win32NT)
                    lokinetExeString = String.Format("{0}\\lokinet-bootstrap.exe", Directory.GetCurrentDirectory());
                else
                    lokinetExeString = String.Format("{0}/lokinet-bootstrap", Directory.GetCurrentDirectory());

                lokinet_bootstrap.StartInfo.UseShellExecute = false;
                lokinet_bootstrap.StartInfo.CreateNoWindow = true;
                lokinet_bootstrap.StartInfo.WorkingDirectory = Directory.GetCurrentDirectory();
                lokinet_bootstrap.StartInfo.FileName = lokinetExeString;
                lokinet_bootstrap.StartInfo.Arguments = string.Format("--cacert rootcerts.pem -L {0} -o {1}", uriBox.Text, default_path);
                lokinet_bootstrap.Start();
                lokinet_bootstrap.WaitForExit();
                if (lokinet_bootstrap.ExitCode == 0)
                    DialogResult = DialogResult.OK;
                else
                    throw (ex); // pass the original exception back to the caller, TLS failed
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("An error occured while downloading data. {0}", ex.Message), "Error", MessageBoxButtons.OK, MessageBoxIcon.Hand);
                DialogResult = DialogResult.Abort;
            }
            wc.Dispose();
            Close();
        }

        private bool cert_check(object sender, X509Certificate cert, X509Chain chain, SslPolicyErrors error)
        { 
        // If the certificate is a valid, signed certificate, return true.
        if (error == System.Net.Security.SslPolicyErrors.None)
        {
            return true;
        }

        MessageBox.Show(string.Format("X509Certificate [{0}] Policy Error: '{1}'",
            cert.Subject,
            error.ToString()), "SSL Error", MessageBoxButtons.OK, MessageBoxIcon.Hand);

        return false;
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }
    }
}
