class DelayedOutput{
  private:
    unsigned long start;
    unsigned long current;
    int pin;
    int value;
  public:
    DelayedOutput(int pin, int value);
    long update();
}
DelayedOutput::DelayedOutput(int pin, int value){
  this->start = millis();
  this->current = this->start;
  Serial.print("new timer created at ");
  Serial.print(this->current);
  Serial.print("\n");
}
