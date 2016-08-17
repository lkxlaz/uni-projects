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
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.scene.control.TableColumn.CellDataFeatures;
import javafx.util.Callback;
import model.*;

/**
 *
 * @author liangze
 */
public class StudentController extends Controller<Student>{
    @FXML private TableView<Activity> myActivitiesTv;
    @FXML private TableView<Activity> uniActivitiesTv;
    @FXML private Button enrolBtn;
    @FXML private Button withdrawBtn;
    @FXML private ComboBox subjectsCb;
    @FXML private Text ScholarshipTxt;
    @FXML private TableColumn<Activity, String> enrolledClm;
    @FXML private TableColumn<Activity, String> uniSubjectClm;
    @FXML private TableColumn<Activity, String> MySubjectClm;
    private Subject currentSubject;
    @FXML private void initialize() {
        ScholarshipTxt.setText(scholarshipValue());
        subjectsCb.valueProperty().addListener(new ChangeListener<Subject>() {
        @Override public void changed(ObservableValue ov, Subject t, Subject t1) {       
            uniActivitiesTv.setItems(t1.getActivities());
            setCurrentSubject(t1);
        }    
    });
        //
        myActivitiesTv.getSelectionModel().selectedItemProperty().addListener(
            (o, oldAc, newAc) -> {
                withdrawBtn.setDisable(newAc == null);
                uniActivitiesTv.setItems(getCurrentSubject().getActivities());}
        );
        uniActivitiesTv.getSelectionModel().selectedItemProperty().addListener(
            (o, oldAc, newAc) -> updateEnrolButton() ); 
	enrolledClm.setCellValueFactory(cellData -> cellData.getValue().enrolledProperty().asString());
        uniSubjectClm.setCellValueFactory(new Callback<CellDataFeatures<Activity, String>, ObservableValue<String>>() {
                @Override
                public ObservableValue<String> call(CellDataFeatures<Activity, String> c) {
                    return new SimpleStringProperty(c.getValue().getSubjectNumber() + "");                
                }        
        });
        MySubjectClm.setCellValueFactory(new Callback<CellDataFeatures<Activity, String>, ObservableValue<String>>() {
                @Override
                public ObservableValue<String> call(CellDataFeatures<Activity, String> c) {
                    return new SimpleStringProperty(c.getValue().getSubjectNumber() + "");                
                }        
        });
    }
    @FXML private void handleEnrol(ActionEvent event) {
        Activity activity = getUniTvSelectedActivity();
        model.enrol(activity);  
        updateEnrolButton();
    } 
    @FXML private void handleWithdraw(ActionEvent event) {
        Activity activity = getMySelectedActivity();
        model.withdraw(activity);
        updateEnrolButton();
    }
    @FXML private void handleLogout(ActionEvent event) {
        stage.close();
    }
    private void updateEnrolButton() {
        Activity activity = getUniTvSelectedActivity();
        enrolBtn.setDisable( !(activity.canEnrol() && (!model.isEnrolledIn(activity)) && activity != null)
                 );
    }
    private String scholarshipValue() {
        boolean temp = getStudent().getScholarship();
        if(temp) return "Yes";
        else return "No";
    }
    private Subject getCurrentSubject() { return currentSubject; }
    private void setCurrentSubject(Subject subject) { currentSubject = subject; }
    public final Student getStudent() { return model; }
    public final University getUniversity() { return getStudent().getUniversity(); }
    private Activity getUniTvSelectedActivity() { return uniActivitiesTv.getSelectionModel().getSelectedItem(); }
    private Activity getMySelectedActivity() { return myActivitiesTv.getSelectionModel().getSelectedItem(); }
}
