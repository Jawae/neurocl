//
//  ViewController.swift
//  myne
//
//  Created by Albert Murienne on 12/03/2017.
//  Copyright © 2017 Blackccpie. All rights reserved.
//

import UIKit

class ViewController: UIViewController, UIImagePickerControllerDelegate, UINavigationControllerDelegate {
    
    @IBOutlet weak var StartCamera: UIButton!
    
    @IBOutlet weak var ImageDisplay: UIImageView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    
    @IBAction func StartCameraAction(_ sender: UIButton) {
        
        if UIImagePickerController.isSourceTypeAvailable(
            UIImagePickerControllerSourceType.camera) {
                
                let picker = UIImagePickerController()
                picker.delegate = self
                picker.sourceType = .camera
        
                present(picker, animated: true, completion: nil)
                
        }
    }
    
    func imagePickerController(_ picker: UIImagePickerController, didFinishPickingMediaWithInfo info: [String : Any]) {
        
        var neuroclWrapper: NeuroclWrapper = NeuroclWrapper()
        
        ImageDisplay.image = info[UIImagePickerControllerOriginalImage] as? UIImage;
        dismiss(animated: true, completion: nil)
        
    }
}

