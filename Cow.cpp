#include "Cow.h"
#include "Utility.h"
#include <memory>

Cow::Cow(shared_ptr<vector<shared_ptr<Animal>>> _environment) :Animal(_environment)
{
	position = RandomPositionVector();
	energy = AnimalConstants::COW_INITIAL_ENERGY;
}

Cow::Cow(shared_ptr<vector<shared_ptr<Animal>>> _environment, Vector2D _position, Gender _gender) :Animal(_environment)
{
	position = _position;
	energy = AnimalConstants::COW_INITIAL_ENERGY;
}

Species Cow::GetSpecies()
{
	return Species::Cow;
}

void Cow::Update()
{
	if (isDead) return;
	age_int += 1;
	prev_position = position;
	if (age_int > AnimalConstants::COW_MAX_AGE || energy <= AnimalConstants::COW_MIN_ENERGY)
		Die();
	Move();
	Breed();
}

Age Cow::GetAge()
{
	return Age(age_int >= AnimalConstants::COW_ADULT_AGE);
}

QString Cow::GetPicturePath()
{
	return ":/sourcePicture/cow.png";
}

void Cow::Mutate()
{
	return;
}

void Cow::Breed()
{
	if (isDead) return;
	if (GetAge() == Age::Child || RandomFloat(0.0, 1.0) > AnimalConstants::COW_BREED_PROBABILITY)
		return;

	shared_ptr<Animal> other = Environment::GetClosetPair(environment, *this, Species::Cow);
	if (!other)
		return;
	if (other->GetGender() != gender)
	{
		bool is_success = false;
		if (other->GetGender() == Gender::Female && other->GetEnergy() > AnimalConstants::COW_INITIAL_ENERGY * 2.0)
		{
			other->DecreaseEnergy(AnimalConstants::COW_INITIAL_ENERGY);
			is_success = true;
		}
		else if (energy > AnimalConstants::COW_INITIAL_ENERGY * 2.0)
		{
			DecreaseEnergy(AnimalConstants::COW_INITIAL_ENERGY);
			is_success = true;
		}
		
		if(is_success)
		{
			shared_ptr<Animal> new_animal =
				std::make_shared<Cow>(this->environment, RandomPositionVector(position, AnimalConstants::BREED_RADIUS), Gender(RandomInteger(0, 1)));
			this->environment->push_back(new_animal);
		}
	}
}

void Cow::Move()
{	
	if (isDead) return;
	MoveState state = MoveState::Walk;
	Vector2D unit_direction;

	shared_ptr<Animal> nearest = Environment::GetClosetPair(environment, *this, { Species::Grass, Species::Wolf, Species::Tiger });

	if (nearest && Vector2D::GetDistance(nearest->GetPosition(), this->GetPosition()) <= AnimalConstants::PROBE_RADIUS)
	{
		if (nearest->GetSpecies() == Species::Grass)
			unit_direction = (nearest->GetPosition() - this->GetPosition()).GetNormalized();
		else
			unit_direction = (this->GetPosition() - nearest->GetPosition()).GetNormalized();
		state = MoveState::Run;
	}

	float velocity_scalar;

	if (state == MoveState::Run)
	{

		if (this->stamina >= AnimalConstants::COW_MAX_STAMINA / 2)
		{
			velocity_scalar = AnimalConstants::COW_MAX_VELOCITY;
		}
		else if (this->stamina > AnimalConstants::COW_MIN_STAMINA)
		{
			velocity_scalar = AnimalConstants::COW_MAX_VELOCITY * (this->stamina * 2 / AnimalConstants::COW_MAX_STAMINA);
		}
		else
		{
			velocity_scalar = AnimalConstants::COW_MIN_VELOCITY;
		}
	}
	else
	{
		if (RandomFloat(0.0, 1.0) <= AnimalConstants::COW_IDLE_PROBABILITY)
		{
			velocity_scalar = 0;
			unit_direction = Vector2D(0, 0);
			state = MoveState::Idle;
		}
		else
		{
			unit_direction = RandomUnitVector();
			velocity_scalar = AnimalConstants::COW_MIN_VELOCITY;
			state = MoveState::Walk;
		}
	}

	//Get Direction
	this->velocity = unit_direction * velocity_scalar;

	//update position
	this->position = this->position + this->velocity;
	this->position.ClipBound();

	//update stamina and energy
	if (state == MoveState::Idle || state == MoveState::Walk)
	{
		this->stamina = std::min(this->stamina + AnimalConstants::COW_RECOVER_STAMINA_RATIO, AnimalConstants::COW_MAX_STAMINA);
		this->energy = std::max(this->energy - AnimalConstants::COW_CONSUME_ENERGY_RATIO, 0.0f);
	}
	else if (state == MoveState::Run)
	{
		this->stamina = std::max(this->stamina - AnimalConstants::COW_CONSUME_STAMINA_RATIO, AnimalConstants::COW_MIN_STAMINA);
	}

	Eat(*nearest);
}

bool Cow::Eat(Animal& other)
{	
	if (isDead) return false;
	if (other.GetSpecies() != Species::Grass)
		return false;
	if ((other.GetPosition() - position).GetLength() <= GetCollisionRadius())
	{
		if(!other.Hurt(AnimalConstants::DAMAGE[int(GetSpecies())]))
		{
			energy += AnimalConstants::DAMAGE[int(GetSpecies())];
			energy = std::min(AnimalConstants::COW_MAX_ENERGY, energy);
			other.Die();
			return true;
		}
	}
	return false;
}
