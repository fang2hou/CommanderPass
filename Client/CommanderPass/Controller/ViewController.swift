//
//  ViewController.swift
//  CommanderPass
//
//  Created by Zhou Fang on 2019/01/05.
//  Copyright © 2019 Zhou Fang. All rights reserved.
//

import Cocoa
import ORSSerial
import Alamofire

enum statusCollection {
    case wait
    case connectToSender
    case senderReady
    case userReady
    case tokenGot
    case tokenSent
    case authKeyGot
    case authKeyVerified
    case authKeyError
}

class ViewController: NSViewController, ORSSerialPortDelegate {
    
    private let API_SERVER = "http://127.0.0.1:8080"
    
    var buffer: String = ""
    var serialPort: SerialPort?
    var serialPortPaths: [String:String] = [:]
    
    var status: statusCollection = .wait { didSet { operation() } }
    
    var userToken: String = ""
    var authKey: String = ""
    
    @IBOutlet weak var serialPortPicker: NSPopUpButton!
    @IBOutlet weak var statusInfoLabel: NSTextField!
    @IBOutlet weak var usernameLabel: NSTextField!
    
    @IBOutlet weak var startButton: NSButton!
    @IBOutlet weak var debugInfo: NSTextField!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        refreshSerialPortList()
    }
    
    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }
    
    @IBAction func detectArduinoButtonPressed(_ sender: NSButton) {
        let portNames = serialPortPicker.itemArray
        for portName in portNames {
            if portName.title.contains("usbmodem") {
                serialPortPicker.select(portName)
            }
        }
        
    }
    @IBAction func refreshButtonPressed(_ sender: Any) {
        refreshSerialPortList()
    }
    
    @IBAction func quitButtonPressed(_ sender: NSButton) {
        NSApplication.shared.terminate(sender)
    }
    
    @IBAction func connectButtonPressed(_ sender: NSButton) {
        guard let selectedPortName = serialPortPicker.selectedItem?.title else {
            updateStatusInfo("Cannot get name of selected port.")
            return
        }
        
        guard let selectedPath = serialPortPaths[selectedPortName] else {
            updateStatusInfo("Cannot get path of selected port.")
            return
        }
        
        updateStatusInfo("Try to connect to \(selectedPortName).")
        connectToArduino(withPath: selectedPath, andBaudrate: 9600)
    }
    
    @IBAction func startButtonPressed(_ sender: Any) {
        status = .connectToSender
    }
    
    func refreshSerialPortList() {
        serialPortPicker.removeAllItems()
        serialPortPaths.removeAll()
        
        let serialPorts = ORSSerialPortManager.shared().availablePorts

        for eachSerialPort in serialPorts {
            serialPortPaths.updateValue(eachSerialPort.path, forKey: eachSerialPort.name)
            serialPortPicker.addItem(withTitle: eachSerialPort.name)
        }
    }
    
    func connectToArduino(withPath path: String, andBaudrate baudrate: NSNumber) {
        if serialPort != nil {
            serialPort!.close()
            serialPort = nil
        }
        
        // set delay for terminating port terminate
        DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
            self.buffer = ""
    
            self.serialPort = SerialPort(withPath: path, andBaudrate: baudrate, delegate: self)
            self.serialPort!.start()
        }
    }
    
    func updateStatusInfo(_ text: String) {
        statusInfoLabel.stringValue = text
    }
    
    // --------------------
    // Serial Port Delegate
    // --------------------
    func serialPortWasRemoved(fromSystem serialPort: ORSSerialPort) {
        updateStatusInfo("Serial port \(serialPort) was removed from macOS.")
    }
    
    func serialPortWasOpened(_ serialPort: ORSSerialPort) {
        updateStatusInfo("Serial port \(serialPort) was opened.")
    }
    
    func serialPort(_ serialPort: ORSSerialPort, didReceive data: Data) {
        if let string = NSString(data: data, encoding: String.Encoding.utf8.rawValue) as String? {
            handleData(withData: string)
        }
    }
    
    // Data handling
    func handleData(withData data: String) {
        if data.contains("\n") {
            buffer += data.replacingOccurrences(of: "\n", with: "")
            filterData(with: buffer)
            buffer = ""
        } else {
            buffer += data
        }
    }
    
    func filterData(with data: String) {
        
        debugInfo.stringValue += "\(data)\n"
        
        
        switch data {
        case "ASenderReady":
            status = .senderReady
        case "AUserReadyViaSender":
            status = .userReady
        case "ATokenGot":
            status = .tokenSent
        default:
            if data.hasPrefix("A#auth#") {
                authKey = data
                authKey = String(authKey.dropFirst("A#auth#".count))
                status = .authKeyGot
            }
        }
    }

    // --------------------
    // API
    // --------------------
    func getToken() {
        let url = API_SERVER + "/get/uuid"
        let params:[String:String] = [
            "username": usernameLabel.stringValue
        ]
        
        Alamofire.request(url, method: .get, parameters: params).responseString { (response) in
            switch response.result {
            case .success(let data):
                self.userToken = data
                self.status = .tokenGot
            case .failure(_):
                print("token request error.")
            }
        }
    }
    
    func sendAuthKey() {
        let url = API_SERVER + "/post/auth"
        let params:[String:String] = [
            "key": authKey
        ]
        
        Alamofire.request(url, method: .post, parameters: params).responseString { (response) in
            switch response.result {
            case .success(let data):
                if data == "success" {
                    self.status = .authKeyVerified
                } else {
                    self.status = .authKeyError
                }
            case .failure(_):
                print("authkey request error.")
            }
        }
    }
    
    func operation() {
        switch status {
        case .wait:
            updateStatusInfo("Waiting...")
        case .connectToSender:
            serialPort?.send(withData: "CWaitSender\n")
            updateStatusInfo("✅ 👷‍♂️ Try to establish connection.")
        case .senderReady:
            serialPort?.send(withData: "CWaitUser\n")
            updateStatusInfo("✅ 🔴 Sender Arduino is ready!")
        case .userReady:
            updateStatusInfo("✅ 🔵 User Arduino is ready! Getting token from server.")
            DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                self.getToken()
            }
        case .tokenGot:
            updateStatusInfo("✅ 👮‍♀️ Got token successfully, sending it to Arduino.")
            serialPort?.send(withData: "C#token#\(userToken)\n")
        case .tokenSent:
            updateStatusInfo("✅ ➡️ Token sent to Arduino. Waiting for input.")
        case .authKeyGot:
            updateStatusInfo("✅ ⬅️ Got authkey from Arduino. Prepared to send.")
            serialPort?.send(withData: "CAuthKeyGot\n")
            DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                self.sendAuthKey()
            }
        case .authKeyVerified:
            updateStatusInfo("✅ 🔓 Authkey has been verified, please check the web page.")
            serialPort?.send(withData: "CAuthSuccess\n")
        case .authKeyError:
            updateStatusInfo("❌ 🔒 Your password is not correct please retry.")
            serialPort?.send(withData: "CAuthFailed\n")
        }
    }
}
