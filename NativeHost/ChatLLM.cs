using Azure.AI.OpenAI;
using OpenAI.Chat;
using System;
using System.ClientModel;
using System.Management.Automation;
using System.Reflection;

namespace NativeHost
{
    public class ChatLLM
    {
        private string? _uri;
        private string? _key;
        private string? _modele;


        //  uri=;key=;modele=;
        private bool extractconnectionString(string connection)
        {
            // extract the connection string
            string[] parts = connection.Split(";");
            if (parts.Length == 3)
            {
                // test if the connection string is well formed

                var uri = parts[0].Split("=")[1];
                var key = parts[1].Split("=")[1];
                var modele = parts[2].Split("=")[1];
                if (uri != null && key != null && modele != null)
                {
                    _uri = uri;
                    _key = key;
                    _modele = modele;
                    return true;
                }
            }
            return false;
        }
        public string RunCommandLLM(string filetoconvert, string connection)
        {
            try
            {
                if (!extractconnectionString(connection))
                {
                    return "Invalid connection string";
                }
                else
                {
                    if (_uri != null && _key != null && _modele != null)
                    {
                        var file = new System.IO.StreamReader(filetoconvert);
                        string line = file.ReadToEnd();
                        file.Close();
                        string command = line;
                        string keyFromEnvironment = _key;
                        AzureOpenAIClient azureClient = new(new Uri(_uri), new ApiKeyCredential(keyFromEnvironment));
                        ChatClient chatClient = azureClient.GetChatClient(_modele);

     
                        ChatCompletion completion = chatClient.CompleteChat(
                                            [
                                                // System messages represent instructions or other guidance about how the assistant should behave
                                                new SystemChatMessage("You are a software engineer and you can only answer with source code. Convert the provided VBScript application code to a PowerShell Core application, maintaining a similar structure and functionality as the given example and using libraries provided. Output the full source code without explanations.\r\nto not add ```PowerShell or ```"),
                                                // User messages represent user input, whether historical or the most recent input
                                                new UserChatMessage(command),
                                                // Assistant messages in a request represent conversation history for responses
                                            ]);
                        return completion.Content[0].Text;
                    }
                    else
                    {
                        return "Invalid connection string";
                    }
                }
            }
            catch (Exception ex)
            {
                return ex.Message;
            }            
        }
    }
}

