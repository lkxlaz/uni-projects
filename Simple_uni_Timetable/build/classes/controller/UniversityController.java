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
import java.util.Iterator;
import model.*;


/**
 *
 * @author liangze
 */
public class UniversityController extends Controller<University> {
    @FXML private ListView<Student> studentsLv;
    @FXML private Button removeBtn;
    @FXML private Button loginBtn;
    
    @FXML private void initialize() {
        studentsLv.setItems(getUniversity().getStudents());
        studentsLv.getSelectionModel().selectedItemProperty().addListener(
            (o, oldStu, newStu) -> {removeBtn.setDisable(newStu == null);
                loginBtn.setDisable(newStu == null);}
        );
                    
    }
    @FXML private void handleAddStudent(ActionEvent event) throws Exception{
        ViewLoader.showStage(model, "/view/add_student.fxml", "Add Student", new Stage());
    }
    @FXML private void handleLogin(ActionEvent event) throws Exception{   
        ViewLoader.showStage(getSelectedStudent(), "/view/student.fxml", "Student", new Stage());
    }
    @FXML private void handleRemove(ActionEvent event) {
        //look up UniActivities
        //for which student enrolled in
        //withdraw it
        Student student = getSelectedStudent();
        for(Subject subject: getUniversity().getSubjects()) {
            for(Iterator<Activity> it = subject.getActivities().listIterator(); it.hasNext() ;) {
                Activity tempAc = it.next();
                if(student.isEnrolledIn(tempAc))
                    student.withdraw(tempAc);
            }
        }
        //remove this student
        getUniversity().remove(student);
    }
    public final University getUniversity() { return model; }
    private Student getSelectedStudent() { return studentsLv.getSelectionModel().getSelectedItem(); }
}
