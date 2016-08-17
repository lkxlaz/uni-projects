package model;

import javafx.beans.property.IntegerProperty;
import javafx.beans.property.ReadOnlyIntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;

public class Activity {
    private Subject subject;
    private String group;
    private int number;
    private String day;
    private int start;
    private int duration;
    private String room;
    private int capacity;
    private IntegerProperty enrolled = new SimpleIntegerProperty();


    public Activity(Subject subject, String group, int number, String day, int start, int duration, String room, int capacity) {
        this.subject = subject;
        this.group = group;
        this.number = number;
        this.day = day;
        this.start = start;
        this.duration = duration;
        this.room = room;
        this.capacity = capacity;
        enrolled.set(0);
    }

    public Subject getSubject() { return subject; }
    public final int getSubjectNumber() { return subject.getNumber(); }
    public final String getGroup() { return group; }
    public final int getNumber() { return number; }
    public final String getDay() { return day; }
    public final int getStart() { return start; }
    public final int getDuration() { return duration; }
    public final String getRoom() { return room; }
    public final int getCapacity() { return capacity; }
    public int getEnrolled() { return enrolled.get(); }
    public ReadOnlyIntegerProperty enrolledProperty() { return enrolled; }

    public boolean canEnrol() {
        return enrolled.get() < capacity;
    }

    public void enrol() {
        enrolled.set(enrolled.get() +1 );
    }

    public void withdraw() {
        enrolled.set(enrolled.get() - 1);
    }

    public boolean matches(int subjectNumber, String group) {
        return subject.matches(subjectNumber) && group.equals(this.group);
    }

    @Override
    public String toString() {
        return String.format("%d %s %d %s %s %02d:00 %dhrs %d/%d", subject.getNumber(), group, number, day, room, start, duration, enrolled, capacity);
    }
}
