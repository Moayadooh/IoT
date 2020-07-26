#include <Fuzzy.h>

// Fuzzy
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput
FuzzySet *low1 = new FuzzySet(0, 0, 10, 25);
FuzzySet *medium1 = new FuzzySet(15, 25, 25, 35);
FuzzySet *high1 = new FuzzySet(25, 40, 50, 50);

// FuzzyInput
FuzzySet *low2 = new FuzzySet(0, 0, 30, 50);
FuzzySet *medium2 = new FuzzySet(30, 50, 50, 70);
FuzzySet *high2 = new FuzzySet(50, 70, 100, 100);

// FuzzyOutput
FuzzySet *less = new FuzzySet(0, 0, 40, 75);
FuzzySet *medium3 = new FuzzySet(50, 100, 100, 150);
FuzzySet *more = new FuzzySet(125, 160, 200, 200);

// Building FuzzyRule
void BuildingFuzzyRule(FuzzySet *cond1, FuzzySet *cond2, FuzzySet *result, int num)
{
  FuzzyRuleAntecedent *fuzzyRuleAntecedent = new FuzzyRuleAntecedent();
  fuzzyRuleAntecedent->joinWithAND(cond1, cond2);

  FuzzyRuleConsequent *fuzzyRuleConsequent = new FuzzyRuleConsequent();
  fuzzyRuleConsequent->addOutput(result);

  FuzzyRule *fuzzyRule = new FuzzyRule(num, fuzzyRuleAntecedent, fuzzyRuleConsequent);
  fuzzy->addFuzzyRule(fuzzyRule);
}

void setup()
{
  Serial.begin(9600);

  // FuzzyInput
  FuzzyInput *temperature = new FuzzyInput(1);

  temperature->addFuzzySet(low1);
  temperature->addFuzzySet(medium1);
  temperature->addFuzzySet(high1);
  fuzzy->addFuzzyInput(temperature);

  // FuzzyInput
  FuzzyInput *soilMoisture = new FuzzyInput(2);

  soilMoisture->addFuzzySet(low2);
  soilMoisture->addFuzzySet(medium2);
  soilMoisture->addFuzzySet(high2);
  fuzzy->addFuzzyInput(soilMoisture);

  // FuzzyOutput
  FuzzyOutput *waterAmount = new FuzzyOutput(1);

  waterAmount->addFuzzySet(less);
  waterAmount->addFuzzySet(medium3);
  waterAmount->addFuzzySet(more);
  fuzzy->addFuzzyOutput(waterAmount);

  // Building FuzzyRule
  BuildingFuzzyRule(high1, high2, less, 1);
  BuildingFuzzyRule(medium1, high2, less, 2);
  BuildingFuzzyRule(low1, high2, less, 3);
  BuildingFuzzyRule(high1, medium2, medium3, 4);
  BuildingFuzzyRule(high1, low2, more, 5);
  BuildingFuzzyRule(medium1, low2, more, 6);
  BuildingFuzzyRule(medium1, medium2, medium3, 7);
  BuildingFuzzyRule(low1, medium2, medium3, 8);
  BuildingFuzzyRule(low1, low2, more, 9);
}

void loop()
{
  float input1 = 50;//random(0, 50);
  float input2 = 100;//random(0, 100);

  Serial.println("\n\n\nEntrance: ");
  Serial.print("\t\t\tTemperature: ");
  Serial.print(input1);
  Serial.print(", Soil Moisture: ");
  Serial.print(input2);

  fuzzy->setInput(1, input1);
  fuzzy->setInput(2, input2);

  fuzzy->fuzzify();

  Serial.println("\nInput: ");
  Serial.print("\tTemperature: High-> ");
  Serial.print(high1->getPertinence());
  Serial.print(", Medium-> ");
  Serial.print(medium1->getPertinence());
  Serial.print(", Low-> ");
  Serial.println(low1->getPertinence());

  Serial.print("\tSoil Moisture: High-> ");
  Serial.print(high2->getPertinence());
  Serial.print(",  Medium-> ");
  Serial.print(medium2->getPertinence());
  Serial.print(",  Low-> ");
  Serial.print(low2->getPertinence());

  float output1 = fuzzy->defuzzify(1);

  Serial.println("\n\nOutput: ");
  Serial.print("\tWater Amount: Less-> ");
  Serial.print(less->getPertinence());
  Serial.print(", Medium-> ");
  Serial.print(medium3->getPertinence());
  Serial.print(", More-> ");
  Serial.println(more->getPertinence());

  Serial.println("\nResult: ");
  Serial.print("\t\t\tWater Amount: ");
  Serial.print(output1);

  // wait 120 seconds
  delay(120000);
}
