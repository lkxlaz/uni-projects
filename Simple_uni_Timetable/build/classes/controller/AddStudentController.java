/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package controller;
import javafx.collections.*;
import javafx.event.*;
import javafx.fxml.*;
import javafx.scene.text.*;
import javafx.scene.control.*;
import javafx.stage.*;
import javafx.beans.property.*;
import java.io.*;
import java.text.*;
import au.edu.uts.ap.javafx.*;
import javafx.beans.binding.BooleanBinding;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.scene.control.ToggleButton;
import model.*;
import model.University.StudentAlreadyExistsException;
/**
 *
 * @author liangze
 */
public class AddStudentController extends Controller<University>{
    @FXML private TextField numberTf;
    @FXML private TextField nameTf;
    @FXML private ToggleGroup attendanceTg;
    @FXML private CheckBox scholarshipCb;
    @FXML private Text errorTxt;
    @FXML private Button addBtn;
    @FXML private RadioButton radioBtn1;
    @FXML private RadioButton radioBtn2;
    private BooleanProperty checkAttendance = new SimpleBooleanProperty();
    
    @FXML private void initialize() {     
       	numberTf.textProperty().addListener((o, oldText, newText) -> updateButtons());
	nameTf.textProperty().addListener((o, oldText, newText) -> updateButtons());
        attendanceTg.selectedToggleProperty().addListener((o, old, now) -> updateButtons());
                              
    }
    @FXML private void handleCancel(ActionEvent event) {
        stage.close();
    }
    @FXML private void handleAdd(ActionEvent event) {
        try {
        //add student to the list
        String number = getNumber();
        String name = getName();
        String attendance = getAttendance();
        boolean scholarship = scholarshipCb.isSelected();
        getUniversity().addStudent(number, name, attendance, scholarship);
        //close the window
        stage.close();
        }
        catch(StudentAlreadyExistsException e) {
            errorTxt.setText("Student already exists");
        }
    }
    
    private void updateButtons() {
        addBtn.setDisable( ! (nameFieldValid() && numberFieldValid() && attendanceFieldValid()));
    }
    private String getAttendance() {
        if (attendanceTg.getSelectedToggle() != null)
             return attendanceTg.getSelectedToggle().getUserData().toString();
        else
             return null;
    }
    private Boolean nameFieldValid() { return getName().length() > 0; }
    private Boolean numberFieldValid() { return getNumber().length() > 0; }
    public final University getUniversity() { return model; }
    private String getNumber() { return numberTf.getText(); }
    private String getName() { return nameTf.getText(); }
    private Boolean attendanceFieldValid () { return  getAttendance() != null; }

   
}
